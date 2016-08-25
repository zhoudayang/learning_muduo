//
// Created by fit on 16-8-25.
//

#ifndef EXCEPTION_H
#define EXCEPTION_H
namespace muduo {
    class Exception : public std::exception {
    public:
        explicit Exception(const char *what);

        explicit Exception(const string &what);

        virtual ~Exception() throw();

        virtual const char *what() const throw();

        const char *stackTrace() const throw();

    private:
        void fillStackTrace();

        string message_;
        string stack_;
    };
}

#endif
