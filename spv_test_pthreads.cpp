#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <pthread.h>
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif
#include <spirv_msl.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wweak-vtables"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wreserved-macro-identifier"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#endif
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#ifdef BUILD_PARALLEL
// Building parallel
#endif

namespace
{

[[nodiscard]] std::size_t default_thread_count() noexcept
{
	const unsigned int hardware_threads = std::thread::hardware_concurrency();
	if (hardware_threads == 0U)
	{
		return 4U;
	}
	return static_cast<std::size_t>(hardware_threads);
}

[[nodiscard]] bool parse_thread_count(const char *text, std::size_t &out) noexcept
{
	if (text == nullptr || text[0] == '\0')
	{
		return false;
	}

	char *end = nullptr;
	errno     = 0;
	const unsigned long long value = std::strtoull(text, &end, 10);
	if (errno != 0 || end == nullptr || *end != '\0')
	{
		return false;
	}
	if (value == 0ULL)
	{
		return false;
	}
	out = static_cast<std::size_t>(value);
	return true;
}

[[nodiscard]] std::string read_text_file(const char *path)
{
	std::ifstream file{path, std::ios::in};
	if (!file)
	{
		throw std::runtime_error{"failed to open file"};
	}

	std::string contents;
	file.seekg(0, std::ios::end);
	const std::ifstream::pos_type end_pos = file.tellg();
	if (end_pos > 0)
	{
		contents.resize(static_cast<std::size_t>(end_pos));
		file.seekg(0, std::ios::beg);
		file.read(contents.data(), static_cast<std::streamsize>(contents.size()));
	}

	if (!file)
	{
		throw std::runtime_error{"failed to read file"};
	}
	return contents;
}

[[nodiscard]] std::vector<std::uint32_t> compile_glsl_fragment_to_spirv(const std::string &glsl_source)
{
	glslang::TShader shader(EShLangFragment);
	const char *      shader_strings[1] = {glsl_source.c_str()};
	shader.setStrings(shader_strings, 1);

	const EShMessages messages =
		static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
	shader.setEnvInput(glslang::EShSourceGlsl, EShLangFragment, glslang::EShClientVulkan, 450);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	if (!shader.parse(GetDefaultResources(), 450, false, messages))
	{
		throw std::runtime_error{std::string{"GLSL parse failed: "} + shader.getInfoLog()};
	}

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messages))
	{
		throw std::runtime_error{std::string{"GLSL link failed: "} + program.getInfoLog()};
	}

	const glslang::TIntermediate *intermediate = program.getIntermediate(EShLangFragment);
	if (intermediate == nullptr)
	{
		throw std::runtime_error{"GLSL intermediate was null"};
	}

	std::vector<std::uint32_t> spirv;
	glslang::SpvOptions        options;
	options.validate = false;
	glslang::GlslangToSpv(*intermediate, spirv, nullptr, &options);
	return spirv;
}

[[nodiscard]] std::string compile_spirv_to_msl(const std::vector<std::uint32_t> &spirv)
{
	spirv_cross::CompilerMSL compiler{spirv};

	spirv_cross::CompilerMSL::Options options = compiler.get_msl_options();
	options.platform                          = spirv_cross::CompilerMSL::Options::Platform::macOS;
	options.msl_version = spirv_cross::CompilerMSL::Options::make_msl_version(2, 4);
	compiler.set_msl_options(options);

	return compiler.compile();
}

class JobSystem final
{
  public:
	explicit JobSystem(std::size_t thread_count, std::size_t thread_stack_bytes)
	    : thread_count_(thread_count), thread_stack_bytes_(thread_stack_bytes)
	{}

	JobSystem(const JobSystem &)            = delete;
	JobSystem &operator=(const JobSystem &) = delete;
	JobSystem(JobSystem &&)                 = delete;
	JobSystem &operator=(JobSystem &&)      = delete;

	template <typename Work>
	void run(Work &&work) const
	{
#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
		struct StartContext final
		{
			std::remove_reference_t<Work> *work = nullptr;
			std::size_t                    worker_index{};
		};

		auto *work_ptr = &work;

		std::vector<pthread_t>    threads(thread_count_);
		std::vector<StartContext> contexts(thread_count_);

		pthread_attr_t attr;
		const int      attr_init_rc = pthread_attr_init(&attr);
		if (attr_init_rc != 0)
		{
			throw std::runtime_error{"pthread_attr_init failed"};
		}

		if (thread_stack_bytes_ != 0U)
		{
			const int stack_rc =
				pthread_attr_setstacksize(&attr, thread_stack_bytes_);
			if (stack_rc != 0)
			{
				pthread_attr_destroy(&attr);
				throw std::runtime_error{"pthread_attr_setstacksize failed"};
			}
		}

		const auto start_routine = [](void *arg) noexcept -> void * {
			auto &ctx = *static_cast<StartContext *>(arg);
			try
			{
				(*ctx.work)(ctx.worker_index);
			}
			catch (...)
			{
			}
			return nullptr;
		};

		std::size_t created_threads = 0U;
		for (std::size_t worker_index = 0U; worker_index < thread_count_; ++worker_index)
		{
			contexts[worker_index].work         = work_ptr;
			contexts[worker_index].worker_index = worker_index;

			const int rc = pthread_create(&threads[worker_index], &attr, start_routine,
			                              &contexts[worker_index]);
			if (rc != 0)
			{
				for (std::size_t join_index = 0U; join_index < created_threads; ++join_index)
				{
					pthread_join(threads[join_index], nullptr);
				}
				pthread_attr_destroy(&attr);
				throw std::runtime_error{"pthread_create failed"};
			}
			created_threads = worker_index + 1U;
		}

		pthread_attr_destroy(&attr);

		for (pthread_t thread : threads)
		{
			const int rc = pthread_join(thread, nullptr);
			if (rc != 0)
			{
				throw std::runtime_error{"pthread_join failed"};
			}
		}
#else
		std::vector<std::thread> threads;
		threads.reserve(thread_count_);

		for (std::size_t worker_index = 0U; worker_index < thread_count_; ++worker_index)
		{
			threads.emplace_back([&work, worker_index] { work(worker_index); });
		}

		for (std::thread &thread : threads)
		{
			thread.join();
		}
#endif
	}

  private:
	std::size_t thread_count_;
	std::size_t thread_stack_bytes_;
};

}        // namespace

int main(int argc, char **argv)
{
	std::size_t thread_count = default_thread_count();
	if (argc >= 2)
	{
		if (!parse_thread_count(argv[1], thread_count))
		{
			std::cerr << "Usage: " << argv[0] << " [thread_count]\n";
			return EXIT_FAILURE;
		}
	}

#ifndef SPV_TEST_FRAG_PATH
#error "SPV_TEST_FRAG_PATH is expected to be defined by CMake."
#endif

	std::string fragment_source;
	try
	{
		fragment_source = read_text_file(SPV_TEST_FRAG_PATH);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Failed to read fragment shader (" << SPV_TEST_FRAG_PATH << "): "
		          << e.what() << '\n';
		return EXIT_FAILURE;
	}

	std::cerr << "Loaded GLSL fragment (" << SPV_TEST_FRAG_PATH
	          << "), bytes=" << fragment_source.size() << '\n';

	if (!glslang::InitializeProcess())
	{
		std::cerr << "glslang::InitializeProcess() failed\n";
		return EXIT_FAILURE;
	}

	std::vector<std::uint32_t> fragment_spirv;
	try
	{
		std::cerr << "Compiling GLSL -> SPIR-V with glslang...\n";
		fragment_spirv = compile_glsl_fragment_to_spirv(fragment_source);
	}
	catch (const std::exception &e)
	{
		glslang::FinalizeProcess();
		std::cerr << "Failed to compile GLSL to SPIR-V: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
	glslang::FinalizeProcess();

	std::cerr << "Compiled SPIR-V words=" << fragment_spirv.size() << '\n';

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
	constexpr std::size_t worker_stack_bytes = 64U * 1024U * 1024U;
#else
	constexpr std::size_t worker_stack_bytes = 0U;
#endif
#elif defined(__SANITIZE_ADDRESS__)
	constexpr std::size_t worker_stack_bytes = 64U * 1024U * 1024U;
#else
	constexpr std::size_t worker_stack_bytes = 0U;
#endif

	const JobSystem job_system{thread_count, worker_stack_bytes};

	std::mutex               cout_mutex;

	const auto work = [&cout_mutex, &fragment_spirv](std::size_t worker_index) {
		std::string msl;
		try
		{
			msl = compile_spirv_to_msl(fragment_spirv);
		}
		catch (const std::exception &e)
		{
			const std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "worker " << worker_index << " compile failed: " << e.what() << '\n';
			return;
		}

		const std::lock_guard<std::mutex> lock(cout_mutex);
		std::cout << "worker " << worker_index << " msl_bytes=" << msl.size() << '\n';
		std::cout << msl << std::endl;
	};

	job_system.run(work);

	std::cout << "threads=" << thread_count << '\n';

	return EXIT_SUCCESS;
}
