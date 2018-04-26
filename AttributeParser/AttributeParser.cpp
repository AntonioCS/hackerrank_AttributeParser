#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <string>

class Attribute {
	std::string m_name;
	std::string m_value;
public:
	Attribute(std::string name, std::string value) : m_name(name), m_value(value) {}

	std::string getName() const noexcept {
		return m_name;
	}

	std::string getValue() const noexcept {
		return m_value;
	}

	void setValue(std::string value) noexcept {
		m_value = value;
	}

	bool operator==(const Attribute &rhs) {
		m_name == rhs.getName();
	}
};
/*
class Token : public std::enable_shared_from_this<Token> {
	using AttrList = std::vector<std::shared_ptr<Attribute>>;
	using Attrptr = std::shared_ptr<Attribute>;

	std::string m_name;
	std::shared_ptr<Token> m_parent = nullptr;
	std::vector<std::shared_ptr<Token>> m_children;
	AttrList m_attributes;
	
public:
	Token(std::string name) : m_name(name) {
	}

	Token(std::string name, std::shared_ptr<Token> parent) : m_name(name), m_parent(parent) {

	}

	void addAttr(std::string name, std::string value) noexcept {
		m_attributes.emplace_back(std::make_shared<Attribute>(name, value));
	}

	std::shared_ptr<Token> addChild(std::string name) {
		auto token = std::make_shared<Token>(name, shared_from_this());
		m_children.emplace_back(token);
		return token;
	}

	std::shared_ptr<Token> getParent() {
		return m_parent;
	}

	std::vector<std::shared_ptr<Token>> &getChildrenList() noexcept {
		return m_children;
	}

	AttrList &getAttrs() {

	}

	Attrptr findAttr(std::string name) {
		auto res = std::find(std::begin(m_attributes), std::end(m_attributes), Attribute{ name, "" });

		if (res != std::end(m_attributes)) {
			return *res;
		}

		return nullptr;
	}


	

	std::string &getName() noexcept {
		return m_name;
	}


};
*/
/*
class Parser {
	std::string &m_code;

	enum class state_type {
		READING,
		READING_TAG_ATTRS,
		READING_ATTR,
		READING_ATTR_VALUE
	};
public:

	std::shared_ptr<Token> interpret() {
		state_type state = state_type::READING;
		std::shared_ptr<Token> root = std::make_shared<Token>("root");
		std::string tmp;

		auto fast_forward_to = [this](int &i, char c) {
			while (m_code[i] != c) {
				i++;
			}
			i++; //go on beyond
		};

		auto capture_until = [this, fast_forward_to](int &i, char c) -> std::string {
			int pos = i;
			fast_forward_to(i, c);
			i--;
			return m_code.substr(pos, i);
		};

		for (int i = 0, max = m_code.size(); i < max; i++) {
			char &c = m_code[i];

			switch (c) {
			case '<':
				if (m_code[i + 1] == '/') {
					fast_forward_to(i, '>');
					root = root->getParent();
				}
				else {
					i++;
					root = root->addChild(capture_until(i, '>'));
					state = state_type::READING_TAG_ATTRS;
				}
				break;
			case '>':
				state = state_type::READING;
				break;
			case '=':
				if (state == state_type::READING_TAG_ATTRS) {
					fast_forward_to(i, '"');
					auto value = capture_until(i, '"');
					root->addAttr(tmp, value);
				}
				break;
			case ' ':
				continue;
			default:
				switch (state) {		
				case state_type::READING_TAG_ATTRS:
					tmp += c;
					break;
				}
			}
		}

		return root;
	}

	Parser(std::string &code) : m_code(code) {
		//interpret();
	}

};
*/

enum class TokenType {
	TAG,
	ATTRIBUTE,
	TAG_END,	
};

class Token {
public:
	TokenType m_type;
	std::string m_name;
	std::string m_value;

	Token(TokenType type, std::string name, std::string value) : m_type(type), m_name(name), m_value(value) {

	}
	Token(TokenType type, std::string name) : Token(type, name, "") {

	}
};

class Manipulator {
	std::string &m_data;
	size_t m_data_index = 0;
	size_t m_data_length = 0;
	char m_stoppedAt;

public:
	Manipulator(std::string &data) : m_data(data), m_data_length(data.length()) {

	}

	char getChar() noexcept {
		if (m_data_index < m_data_length) {
			return m_data[m_data_index++];
		}

		return EOF;
	}

	char getNextChar() noexcept {
		char c = getChar();
		goBack();

		return c;
	}

	void fastForwardTo(std::vector<char> cTo) {
		char c;
		bool running = true;

		while (running && (c = getChar()) != EOF) {
			for (const auto &i : cTo) {
				if (i == c) {
					m_stoppedAt = c;
					running = false;
					break;
				}
			}
		}
	}

	const char &stoppedAtChar() {
		return m_stoppedAt;
	}

	void fastForwardTo(char cTo) {
		fastForwardTo(std::vector<char>{ cTo });
	};	

	void skipWhiteSpace() {
		char c = getChar();
		if (c != ' ') {
			goBack();
		}
		else {
			while (c == ' ' && c != EOF)
				c = getChar();
		}
		goBack(); //so that next call to getChar gets the no empty value
	}
	
	std::string captureUntil(std::vector<char> cUntil) {
		size_t pos = m_data_index;
		fastForwardTo(cUntil);
		goBack();
		return m_data.substr(pos, m_data_index - pos);
	}

	std::string captureUntil(char cUntil) {
		return captureUntil(std::vector<char>{cUntil});
	};

	void goBack(int i = 1) noexcept {
		if (m_data_index - i >= 0) {
			m_data_index -= i;
		}
	}

	bool isAtEOF() const noexcept {
		return (m_data_index >= m_data_length);
	}
};

class Lexer {
public:
	std::vector<Token> m_tokens;
private:
	std::string m_code;
	Manipulator m_codeManipulator;

	std::string getTagName(std::string data) {

	}	

	std::vector<std::string> getAttributes(std::string data) {

	}

	void processTagData(std::string data) {
		bool tagNameSearch = true;
		Manipulator m{ data };
		std::string tagName = m.captureUntil({ ' ', '>' });
		
		m_tokens.emplace_back(TokenType::TAG, tagName);

		if (m.stoppedAtChar() == ' ') {
			while (true) {
				if (m.isAtEOF()) {
					break;
				}

				m.skipWhiteSpace();

				std::string attrName = m.captureUntil({ ' ', '=', '>' });
				std::string attrValue;
				const char &c = m.stoppedAtChar();

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
	Lexer(std::string &code) : m_code(code), m_codeManipulator{ m_code } {
	}

	void Lex() {
		char c;

		while((c = m_codeManipulator.getChar()) != EOF) {
			if (c == '<') {
				if (m_codeManipulator.getNextChar() == '/') {
					m_codeManipulator.getChar(); //skip /
					auto tagName = m_codeManipulator.captureUntil('>');
					m_tokens.emplace_back(TokenType::TAG_END, tagName);
				}
				else {
					auto tagHeaderData = m_codeManipulator.captureUntil('>');
					processTagData(tagHeaderData);
				}
			}
		}

	}
};

int main() {

	std::string code = R"("<tag1 value = "HelloWorld"><tag2 name = "Name1"></tag2></tag1>")";

	//Manipulator m{ code };

	//std::string res = m.captureUntil(' ');
	//std::cout << res << "--\n";
	//std::cout << m.stoppedAtChar() << "<<" << '\n';
	

	Lexer l{ code };
	l.Lex();

	for (const auto &t : l.m_tokens) {
		switch (t.m_type) {
			case TokenType::TAG:
				std::cout << "Tag start: " << t.m_name << '\n';
			break;
			case TokenType::ATTRIBUTE:
				std::cout << "Attribute: " << t.m_name << '=' << t.m_value << '\n';
			break;
			case TokenType::TAG_END:
				std::cout << "Tag end: " << t.m_name << '\n';
			break;
		}
	}


	//std::string code{ "<tag1></tag1>" };
	/*
	std::shared_ptr<Token> t = std::make_shared<Token>("root");
	std::cout << t->getName() << '\n';

	t->addAttr("Test1", "Bla");
	
	
	std::shared_ptr<Token> child = t->addChild("Teste");
	std::cout << child->getName() << '\n';
	std::cout << child->getParent()->getName() << '\n';
	*/

	//auto ref = t->addChild("Teste");	
	//ref->addAttr("Attr1", "Value1");

	//std::cout << ref->getParent()->getName();

	//Parser p{ code };

	//auto res = p.interpret();

	//std::cout << res->getName();

	//auto res2 = res->getChildrenList();

	//for (auto &token : res2) {
		//std::cout << token->getName() << '\n';
	//}


	//std::cout << code;
	std::cin.get();

	return 0;
}
