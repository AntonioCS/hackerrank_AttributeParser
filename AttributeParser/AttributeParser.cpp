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

class Tag {
	std::vector<Attribute> m_attributes;
	std::vector<Tag> m_children;
	std::string m_name;
public:
	Tag(std::string name) : m_name(name) {

	}

	const std::string &getName() const noexcept {
		return m_name;
	}

	void addAttribute(Attribute newAttr) {
		m_attributes.push_back(newAttr);
	}

	void addChild(Tag tag) {
		m_children.push_back(tag);
	}

	const Tag *getChild(const std::string &tagName) const noexcept {
		return getItem<Tag>(m_children, tagName);
	}

	const Attribute *getAttribute(const std::string &attrName) const noexcept {
		return getItem<Attribute>(m_attributes, attrName);
	}

private:
	template <typename T>
	const T *getItem(const std::vector<T> vec, const std::string &name) const noexcept {
		auto res = std::find_if(vec.begin(), vec.end(), [&name](T item) {
			return item.getName() == name;
		});

		if (res != vec.end()) {
			return &(*res);
		}

		return nullptr;
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
	Tag root("root");
	Tag tag1("Tag1");
	root.addChild(tag1);

	auto t = root.getChild("Tag2");

	if (t) {
		std::cout << t->getName() << '\n';
	}
	else {
		std::cout << "Tag not found\n";
	}


//	std::string code = R"("<tag1 value = "HelloWorld"><tag2 name = "Name1"></tag2></tag1><tag3></tag3>")";

	//Manipulator m{ code };

	//std::string res = m.captureUntil(' ');
	//std::cout << res << "--\n";
	//std::cout << m.stoppedAtChar() << "<<" << '\n';
	

	//Lexer l{ code };
	//l.Lex();
	/*
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
	*/

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
