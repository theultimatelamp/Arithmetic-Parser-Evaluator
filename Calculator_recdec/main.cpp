#include <iostream>
#include <string>
#include <iterator>
#include <memory>
#include <exception>
#include <algorithm>
#include <functional>
#include <cmath>
#include <iomanip>

//this class holds the value of the actua

//the actual parser
//template is used to allow for any return type
template <class FLOAT_TYPE=double>
class Parser
{
private:
    //the string to be parsed
    std::string m_stream;
    //built-in iterator to tell where the parser is looking when parsing
    std::string::iterator m_iter = m_stream.begin();
    //static const string holding the possible digits
    const std::string s_digits = "0123456789";
    //dummy variables for secondary calls
    FLOAT_TYPE m_dummy;

public:
    Parser(const std::string& stream):
        //set the internal string to the input string
        m_stream(stream)
    {
    }

    //check if a char is in a string
    static bool in_string(char c, const std::string& str)
    {
        //don't even try
        if(str.empty())
            return 0;
        //using iterators, check if the char is any of the values in the string
        for(std::string::const_iterator i=str.begin(); i != str.end(); ++i)
            if(c == *i) return 1;
        //return false if none of the tests were positive
        return 0;
    }

    //remove the whitespace from a string
    void remove_whitespace()
    {
        //don't put in the extra effort
        if(m_stream.empty())
            return;

        //use remove_if to shuffle the string and remove the extra junk
        m_stream.erase(std::remove_if(m_stream.begin(), m_stream.end(), isspace), m_stream.end());

        //set the iterator to the beginning of the modified string
        m_iter = m_stream.begin();
    }

    //can print the string
    friend std::ostream& operator<<(std::ostream& out, Parser parse)
    {
        //push the string into the ostream
        out << parse.m_stream;
        return out;
    }

private:
    std::string::iterator get_start()
    {
        return m_stream.begin();
    }
    std::string::iterator get_end()
    {
        return m_stream.end();
    }
    //converts a character into an integer
    FLOAT_TYPE to_int(char c)
    {
        //kind of hacky conversion
        return static_cast<FLOAT_TYPE>(c-48);
    }
    //evaluates an integer value
    FLOAT_TYPE evaluate_int(std::string::iterator start_iter, std::string::iterator end_iter)
    {
        //record the length of the substring to calculate the value
        int distance = std::distance(start_iter, end_iter);
        //iterator to hold the place in the string
        std::string::iterator place = start_iter;
        //store the sum of the value
        FLOAT_TYPE sum=0;
        //add all of the values times the appropriate power of ten
        for(int i=0; i<distance; ++i)
        {
            sum += std::pow(10, distance-i-1) * to_int(*place);
            ++place;
        }
        return sum;
    }

    //evaluates a float type
    FLOAT_TYPE evaluate_float(std::string::iterator start_iter, std::string::iterator end_iter)
    {
        //iterator to store the location of the radix point
        std::string::iterator radix_loc;
        //loop until the point is found
        for(std::string::iterator i=start_iter; i != end_iter; ++i)
        {
            //it is assumed that the portion has already been evaluated to be of float structure
            if(*i == '.')
                radix_loc = i;
        }
        //hold the sum
        FLOAT_TYPE sum=0;
        //hold the amount of digits after the radix point
        int distance = std::distance(radix_loc+1, end_iter);
        //calculates the stuff both before and after the point
        sum = evaluate_int(start_iter, radix_loc) + evaluate_int(radix_loc+1, end_iter)/(std::pow(10,distance));
        return sum;
    }
    /*
    *the grammar that describes the arithmetic expression is as follows:
    *
    *expression ::= sub | - term + sub | - term - sub
    *sub        ::= term | term + sub | term negsub
    *negsub     ::= - term + sub | - term negsub | - term
    *term       ::= factor | factor * term | factor / term
    *divterm    ::= / factor * term | / factor divterm | / factor
    *factor     ::= number | ( expression )
    *number     ::= int | int . int
    *int        ::= digit | digit int
    *digit      ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
    *
    *warning: this is not proper notation, but works well enough
    *
    *
    *the parser first checks to make sure the string is an expression, then evaluates it as it parses
    *a reference is passed into each function to hold the result of being evaluated
    */

    //test for the given char
    bool is_char(char c)
    {
        //if the value is a character, then it's fine
        if(*(m_iter++) == c)
            return 1;
        //if not, go back with the char and return false
        --m_iter;
        return 0;
    }

    //test for a digit
    bool is_digit()
    {
        //test if the char at the iterator is a digit
        if(in_string(*(m_iter++), s_digits))
            return 1;
        //if not, reset the iterator
        --m_iter;
        //and return false
        return 0;
    }

    //test for an int
    bool is_int()
    {
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a digit, followed by an int value
        if(is_digit() && is_int())
            return 1;
        //make sure to reset before each test
        m_iter = temp;
        //test if the value is just a digit
        if(is_digit())
            return 1;
        //reset the iterator and return false
        m_iter = temp;
        return 0;
    }

    //overload meant to push a value into a reference
    bool is_int(FLOAT_TYPE& get)
    {
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a digit, followed by an int value
        if(is_digit() && is_int())
        {
            //set the value of the reference to the value of the integer
            get = evaluate_int(temp, m_iter);
            return 1;
        }
        //make sure to reset before each test
        m_iter = temp;
        //test if the value is just a digit
        if(is_digit())
        {
            //again, get the value
            get = evaluate_int(temp, m_iter);
            return 1;
        }
        //reset the iterator and return false
        m_iter = temp;
        return 0;
    }

    //test for a number, which is an int or float
    bool is_number(FLOAT_TYPE& get)
    {
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is an int, followed by a dot, followed by an int
        if(is_int() && is_char('.') && is_int())
        {
            //get the value as a float
            get = evaluate_float(temp, m_iter);
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is just an int, setting it to an int
        if(is_int(get))
            return 1;
        //reset and return false otherwise
        m_iter = temp;
        return 0;
    }

    //test for a factor
    bool is_factor(FLOAT_TYPE& get)
    {
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is just a number
        if(is_number(get))
            return 1;
        //reset before each test
        m_iter = temp;
        //test if the value is an expression surrounded by parentheses
        if(is_char('(') && is_exp(get) && is_char(')'))
            return 1;
        //reset and return false otherwise
        m_iter = temp;
        return 0;
    }

    //test for a divided term
    bool is_divterm(FLOAT_TYPE& get)
    {
        //temp values for testing
        FLOAT_TYPE left, right;
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a divided factor times a term
        if(is_char('/') && is_factor(left) && is_char('*') && is_term(right))
        {
            get = right/left;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is a divided factor again divided
        if(is_char('/') && is_factor(left) && is_divterm(right))
        {
            get = right/left;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is just a dividedc factor
        if(is_char('/') && is_factor(left))
        {
            get = 1/left;
            return 1;
        }
        //if wrong, reset and return false
        m_iter = temp;
        return 0;
    }

    //test for a term
    bool is_term(FLOAT_TYPE& get)
    {
        //temp values for testing
        FLOAT_TYPE left, right;
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a factor times a term
        if(is_factor(left) && is_char('*') && is_term(right))
        {
            //return the correct value
            get = left * right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is a factor over a term
        if(is_factor(left) && is_divterm(right))
        {
            //give the correct value
            get = left * right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is just a factor
        if(is_factor(get))
            return 1;
        //otherwise, just reset and return false
        m_iter = temp;
        return 0;
    }

    //test for a subexpression starting with a negative term
    bool is_negsub(FLOAT_TYPE& get)
    {
        //temp values for testing
        FLOAT_TYPE left, right;
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a negative term plus a subexpression
        if(is_char('-') && is_term(left) && is_char('+') && is_sub(right))
        {
            get = -left + right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is a negative term plus another negsub
        if(is_char('-') && is_term(left) && is_negsub(right))
        {
            get = -left + right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is just a negative term
        if(is_char('-') && is_term(left))
        {
            get = -left;
            return 1;
        }
        //otherwise, just reset and return false
        m_iter = temp;
        return 0;
    }

    //test for a subexpression
    bool is_sub(FLOAT_TYPE& get)   //also stores whether or not the value is negative
    {
        //temp values for testing
        FLOAT_TYPE left, right;
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a term plus a subexpression
        if(is_term(left) && is_char('+') && is_sub(right))
        {
            get = left + right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is a term minus a subexpression
        if(is_term(left) && is_negsub(right))
        {
            get = left + right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is just a term
        if(is_term(get))
            return 1;
        //otherwise, reset and return false
        m_iter = temp;
        return 0;
    }

    //test for an expression
    bool is_exp(FLOAT_TYPE& get)
    {
        //temp values for testing
        FLOAT_TYPE left, right;
        //to reset if false
        std::string::iterator temp = m_iter;
        //test if the value is a negative term plus a subexpression
        if(is_char('-') && is_term(left) && is_char('+') && is_sub(right))
        {
            get = -left + right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is a negative term minus a subexpression
        if(is_char('-') && is_term(left) && is_negsub(right))
        {
            get = -left + right;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is a plain negative subexpression
        if(is_negsub(left))
        {
            get = left;
            return 1;
        }
        //reset before each test
        m_iter = temp;
        //test if the value is just a subexpression
        if(is_sub(get))
            return 1;
        //otherwise, reset and return false
        m_iter = temp;
        return 0;
    }

public:
    //wrapper for is_exp so that it knows if there is crap after any expression
    bool expression(FLOAT_TYPE& get)
    {
        //start by removing whitespace
        remove_whitespace();
        //if it finds an expression, and it is at the end of the string, then it has succeeded
        if(is_exp(get) && m_iter>=m_stream.end())
            return 1;
        //otherwise, something is wrong
        return 0;
    }
};

void calculator()
{
    while(1)
    {
        std::string char_stream;
        getline(std::cin, char_stream);
        if(char_stream == "exit")
            break;
        Parser<double> parse(char_stream);
        parse.remove_whitespace();
        std::cout << parse << "\n";
        double x;
        if(parse.expression(x))
        {
            std::cout << x << "\n";
        }
        else
        {
            std::cout << "failure\n";
        }
    }

}

int main()
{
    calculator();

    return 0;
}
