#include <memory>
#include <functional>
#include <fstream>
#include <ctime>
#include <chrono>

namespace details {
    static unsigned long long COUNTER = 0;
}

template <typename T>
class shell;

template <typename Ret, typename... Params>
class shell<Ret(Params...)> {

public:
    template <typename FunctionObject>
    shell(FunctionObject fo, std::string filename)
        : callable{std::make_unique<callable_impl<FunctionObject>>(std::move(fo))},
        Filename(filename)
    {}

    Ret operator()(Params... params) {
        Ret res = callable->call(params...);
        ++details::COUNTER;
        std::fstream outfile;
        outfile.open(Filename, outfile.out | std::ios::app);
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        outfile << ctime(&t) << ' ';
        outfile << details::COUNTER << ". Function " << typeid(*this).name() << " called with arguments: (";
        print_args(outfile, params...);
        outfile << ')' << " Result: " << res << '\n';
        outfile.close();
        outfile.clear();
        return res;
    }

    shell(shell&& other) : callable(std::move(other.callable)), Filename(std::move(other.Filename)) {}
    ~shell() {};


private:
    
    void print_args(std::fstream& outfile) {}

    template <typename T> void print_args(std::fstream& outfile, const T& t) {
        outfile << t;
    }
    
    template <typename First, typename... Rest>
    void print_args(std::fstream& outfile, const First& first, const Rest&... rest) {
        outfile << first << ", ";
        print_args(outfile, rest...); 
    }


    struct callable_interface {
        virtual Ret call(Params...) = 0;
        virtual void InternalCopy(const callable_interface& other) = 0;
        virtual ~callable_interface() = default;
    };

    template <typename Callable>
    struct callable_impl : callable_interface {
        callable_impl(Callable callable_)
            : callable{std::move(callable_)}
        {}
        Callable callable;
        void InternalCopy(const callable_interface& other) override {}
        Ret call(Params... params) override {
            return std::invoke(callable, params...);
        }
    };


    std::unique_ptr<callable_interface> callable;

public:

    mutable std::string Filename;
    void changeFilename(const std::string& newFilename) const {
        Filename = newFilename;
    }
    
};