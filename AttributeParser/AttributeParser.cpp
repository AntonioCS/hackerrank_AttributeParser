#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Manipulator {
    std::string& m_data;
    size_t m_data_index = 0;
    size_t m_data_length = 0;
    char m_stoppedAt;

public:
    Manipulator(std::string& data)
        : m_data(data)
        , m_data_length(data.length())
    {
    }

    char getChar() noexcept
    {
        if (m_data_index < m_data_length) {
            return m_data[m_data_index++];
        }

        return EOF;
    }

    char getNextChar() noexcept
    {
        char c = getChar();
        goBack();

        return c;
    }

    void fastForwardTo(std::vector<char> cTo)
    {
        char c;
        bool running = true;

        while (running && (c = getChar()) != EOF) {
            for (const auto& i : cTo) {
                if (i == c) {
                    m_stoppedAt = c;
                    running = false;
                    break;
                }
            }
        }
    }

    const char& stoppedAtChar()
    {
        return m_stoppedAt;
    }

    void fastForwardTo(char cTo)
    {
        fastForwardTo(std::vector<char>{ cTo });
    };

    void skipWhiteSpace()
    {
        char c = getChar();
        if (c != ' ') {
            goBack();
        } else {
            while (c == ' ' && c != EOF) {
                c = getChar();
            }
            goBack(); //so that next call to getChar gets the no empty value
        }
    }

    std::string captureUntil(std::vector<char> cUntil)
    {
        size_t pos = m_data_index;
        fastForwardTo(cUntil);
        goBack();
        return m_data.substr(pos, m_data_index - pos);
    }

    std::string captureUntil(char cUntil)
    {
        return captureUntil(std::vector<char>{ cUntil });
    };

    void goBack(int i = 1) noexcept
    {
        if (m_data_index - i >= 0) {
            m_data_index -= i;
        }
    }

    bool isAtEOF() const noexcept
    {
        return (m_data_index >= m_data_length);
    }
};

namespace instructions {
std::vector<std::string> explode(std::string str, char s)
{
    auto startAt = std::begin(str);
    std::vector<std::string> result;
    auto extract = [&str, &startAt, &result](auto m) {
        auto pos = startAt - std::begin(str);
        auto len = (m - std::begin(str)) - pos;

        if (m != std::end(str)) {
            startAt = ++m;
        }

        result.push_back(str.substr(pos, len));
    };

    while (true) {
        auto m = std::find(startAt, std::end(str), s);
        extract(m);
        if (m == std::end(str)) {
            break;
        }
    }

    return result;
}
}

namespace language {

class Attribute {
    std::string m_name;
    std::string m_value;

public:
    Attribute(std::string name, std::string value)
        : m_name(name)
        , m_value(value)
    {
    }

    std::string getName() const noexcept
    {
        return m_name;
    }

    std::string getValue() const noexcept
    {
        return m_value;
    }

    void setValue(std::string value) noexcept
    {
        m_value = value;
    }
    /*
		bool operator==(const Attribute& rhs)
		{
		m_name == rhs.getName();
		}*/
};

class Tag {
    std::vector<Attribute> m_attributes;
    std::vector<std::unique_ptr<Tag>> m_children;
    std::string m_name;
    Tag* m_parent = nullptr;

public:
    Tag(std::string name, Tag* parent)
        : m_name(name)
        , m_parent(parent)
    {
    }

    Tag(std::string name)
        : Tag(name, nullptr)
    {
    }

    void setParent(Tag* parent)
    {
        m_parent = parent;
    }

    Tag* getParent() const noexcept
    {
        return m_parent;
    }

    const std::string& getName() const noexcept
    {
        return m_name;
    }

    void addAttribute(std::string name, std::string value)
    {
        m_attributes.emplace_back(name, value);
    }

    Tag* addChild(std::unique_ptr<Tag>&& childTag)
    {
        childTag->setParent(this);
        m_children.push_back(std::move(childTag));

        return m_children.back().get();
    }

    Tag* getChild(const std::string& tagName) const noexcept
    {
        auto res = std::find_if(m_children.begin(), m_children.end(), [&tagName](const std::unique_ptr<Tag>& item) {
            return item->getName() == tagName;
        });
        return res != m_children.end() ? (*res).get() : nullptr;

        //return getItem<std::unique_ptr<Tag>, Tag>(m_children, tagName);
    }

    const Attribute* getAttribute(const std::string& attrName) const noexcept
    {
        auto res = std::find_if(m_attributes.begin(), m_attributes.end(), [&attrName](auto item) {
            return item.getName() == attrName;
        });
        return res != m_attributes.end() ? &(*res) : nullptr;

        //return getItem<Attribute>(m_attributes, attrName);
    }

    void outputChildren() const noexcept
    {
        for (const auto& i : m_children) {
            std::cout << i->getName() << '\n';
        }
    }
};

enum class TokenType {
    TAG,
    ATTRIBUTE,
    TAG_END
};

class Token {
public:
    TokenType m_type;
    std::string m_name;
    std::string m_value;

    Token(TokenType type, std::string name, std::string value)
        : m_type(type)
        , m_name(name)
        , m_value(value)
    {
    }
    Token(TokenType type, std::string name)
        : Token(type, name, "")
    {
    }
};

class Lexer {
public:
    std::vector<Token> m_tokens;

private:
    std::string m_code;
    Manipulator m_codeManipulator;

    void removeSpace(std::string& str) const noexcept
    {
        str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
    }
    void processTagData(std::string data)
    {
        Manipulator m{ data };
        std::string tagName = m.captureUntil(' ');

        m_tokens.emplace_back(TokenType::TAG, tagName);

        if (m.stoppedAtChar() == ' ') {
            while (true) {
                if (m.isAtEOF()) {
                    break;
                }

                m.skipWhiteSpace();

                std::string attrName = m.captureUntil({ ' ', '=', '>' });
                std::string attrValue;
                const char& c = m.stoppedAtChar();

                if (c == '>') {
                    break;
                }

                switch (c) {
                case ' ':
                    m.skipWhiteSpace();
                    //[[fallthrough]];
                case '=':
                    m.fastForwardTo('"');
                    attrValue = m.captureUntil('"');
                    break;
                }

                m_tokens.emplace_back(TokenType::ATTRIBUTE, attrName, attrValue);
                //we are now at the " we must move past
                m.getChar();
            }
        }
    }

public:
    Lexer(std::string& code)
        : m_code(code)
        , m_codeManipulator{ m_code }
    {
    }

    void Lex()
    {
        char c;
        auto hasNoWhiteSpace = [](std::string tData) {
            return tData.find(' ') == std::string::npos;
        };

        while ((c = m_codeManipulator.getChar()) != EOF) {
            if (c == '<') {
                m_codeManipulator.skipWhiteSpace();
                auto nextChar = m_codeManipulator.getNextChar();

                if (nextChar == '/') {
                    m_codeManipulator.getChar(); //skip /
                    auto tagName = m_codeManipulator.captureUntil('>');
                    removeSpace(tagName);
                    m_tokens.emplace_back(TokenType::TAG_END, tagName);
                } else {
                    auto tagHeaderData = m_codeManipulator.captureUntil('>');
                    //no spaces means its just <tag>
                    if (hasNoWhiteSpace(tagHeaderData)) {
                        m_tokens.emplace_back(TokenType::TAG, tagHeaderData);
                    } else {
                        processTagData(tagHeaderData);
                    }
                }
            }
        }
    }

    const std::vector<Token>& getTokens() const noexcept
    {
        return m_tokens;
    }
};

class Parser {
    const std::vector<Token>& m_tokens;
    Tag m_root{ "root" };

public:
    Parser(const std::vector<Token>& tokens)
        : m_tokens(tokens)
    {
    }

    void parse()
    {
        Tag* current = &m_root;
        //Note: The code is guaranteed to be correct
        for (const Token& token : m_tokens) {
            switch (token.m_type) {
            case TokenType::TAG: {
                auto res = current->addChild(std::make_unique<Tag>(token.m_name));
                current = res;
            } break;

            case TokenType::TAG_END:
                current = current->getParent();
                break;

            case TokenType::ATTRIBUTE:
                current->addAttribute(token.m_name, token.m_value);
                break;
            }
        }
    }

    Tag* getRoot() noexcept
    {
        return &m_root;
    }
};
} // namespace language

void readDataFromCin(int& code_lines, int& query_lines, std::string& code, std::vector<std::string>& queries)
{
    std::cin >> code_lines >> query_lines;

    while (code_lines-- >= 0) {
        std::string tmp;
        std::getline(std::cin, tmp);
        code += tmp;
    }

    //std::cout << code << '\n';

    while (query_lines--) {
        std::string tmp;
        std::getline(std::cin, tmp);
        queries.emplace_back(tmp);
    }

    //for (const auto &i : queries) {
    //std::cout << i << '\n';
    //}
}

void fakeData(std::string& code, std::vector<std::string>& queries)
{
    code = R"(<tag1 v1 = "123" v2 = "43.4" v3 = "hello">
		< / tag1>
		<tag2 v4 = "v2" name = "Tag2">
		<tag3 v1 = "Hello" v2 = "World!">
		< / tag3>
		<tag4 v1 = "Hello" v2 = "Universe!">
		< / tag4>
		< / tag2>
		<tag5>
		<tag7 new_val = "New">
		< / tag7>
		< / tag5>
		<tag6>
		<tag8 intval = "34" floatval = "9.845">
		< / tag8>
		< / tag6>")";

    /*queries.push_back("tag1~v1");
    queries.push_back("tag1~v2");
    queries.push_back("tag1~v3");
    queries.push_back("tag4~v2"); //Not Found!
    queries.push_back("tag2.tag4~v1");
    queries.push_back("tag2.tag4~v2");
    queries.push_back("tag2.tag3~v2");*/
    queries.push_back("tag5.tag7~new_val");
    /*queries.push_back("tag5~new_val");
    queries.push_back("tag7~new_val");
    queries.push_back("tag6.tag8~intval");
    queries.push_back("tag6.tag8~floatval");
    queries.push_back("tag6.tag8~val");
    queries.push_back("tag8~intval");*/
}

void fakeDataTeste01(std::string& code, std::vector<std::string>& queries)
{
    code = R"(
	<a value = "GoodVal">
		<b value = "BadVal" size = "10">
		</b>
		<c height = "auto">
			<d size = "3">
				<e strength = "2">
				</e>
			</d>
		</c>
	</a>)";

    /*queries.push_back("a~value");
    queries.push_back("b~value");
    queries.push_back("a.b~size");
    queries.push_back("a.b~value");
    queries.push_back("a.b.c~height");*/
    queries.push_back("a.c~height");
    /*queries.push_back("a.d.e~strength");
    queries.push_back("a.c.d.e~strength");
    queries.push_back("d~sze");
    queries.push_back("a.c.d~size");*/
}

void outputLexerTokens(const std::vector<language::Token>& tokens)
{
    auto printType = [](const language::Token& t) {
        switch (t.m_type) {
        case language::TokenType::TAG:
            std::cout << "Tag start ";
            break;
        case language::TokenType::TAG_END:
            std::cout << "Tag end ";
            break;
        default:
            std::cout << "Attribute ";
            break;
        }

        std::cout << t.m_name;

        if (t.m_value != "") {
            std::cout << " - " << t.m_value;
        }

        std::cout << '\n';

    };

    for (const auto& t : tokens) {
        printType(t);
    }
}

int main()
{
    int code_lines{ 0 };
    int query_lines{ 0 };
    std::string code{ "" };
    std::vector<std::string> queries;

    readDataFromCin(code_lines, query_lines, code, queries);
    //fakeData(code, queries);
    //fakeDataTeste01(code, queries);

    language::Lexer l{ code };
    l.Lex();

    auto result = l.getTokens();
    /*
    outputLexerTokens(result);
    std::cin.get();
    return 0;
    //*/
    language::Parser p{ result };
    p.parse();

    auto hasTilde = [](std::string str) -> bool {
        return std::find(std::begin(str), std::end(str), '~') != std::end(str);
    };

    for (const auto& q : queries) {
        auto r = p.getRoot();

        //r->outputChildren();

        auto res = instructions::explode(q, '.');

        for (const auto& t : res) {
            if (hasTilde(t)) {
                auto res2 = instructions::explode(t, '~');
                auto thetag = r->getChild(res2[0]);

                if (thetag) {
                    //std::cout << "Found tag\n";
                    auto attr = thetag->getAttribute(res2[1]);

                    if (attr) {
                        std::cout << attr->getValue() << '\n';
                        break;
                    }
                }
                std::cout << "Not Found!\n";
                break;

            } else {
                auto thetag = r->getChild(t);
                if (thetag != nullptr) {
                    r = thetag;
                } else {
                    std::cout << "Not Found!\n";
                    break;
                }
            }
        }
    }
    std::cin.get();
    return 0;
}
