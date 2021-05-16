#include "json.h"
#include <cmath>
#include <iomanip>

using namespace std;

namespace Json {

    ostream& operator << (ostream& output, const Node& node) {
        if (holds_alternative<vector<Node>>(node)) {
            output << '[';
            const vector<Node>& v = node.AsArray();
            if (!v.empty()) output << '\n';
            for (auto it = v.cbegin(); it != v.cend(); ++it) {
                output << *it;
                if (next(it) != v.cend()) output << ',';
                output << '\n';
            }
            output << ']';
        } else if (holds_alternative<map<string, Node>>(node)) {
            output << '{';
            const map<string, Node>& m = node.AsMap();
            if (!m.empty()) output << '\n';
            for (auto it = m.cbegin(); it != m.cend(); ++it) {
                output << "\"" << it->first << "\": " << it->second;
                if (next(it) != m.cend()) output << ',';
                output << '\n';
            }
            output << '}';
        } else if (holds_alternative<string>(node)) {
            output << '"' << node.AsString() << '"';
        } else if (holds_alternative<bool>(node)) {
            output << (node.AsBool() ? "true" : "false");
        } else if (holds_alternative<int>(node)) {
            output << node.AsInt();
        } else if (holds_alternative<double>(node)) {
            output << setprecision(6) << node.AsDouble();
        } else {
            throw runtime_error("invalid type of node");
        }
        return output;
    }

  Document::Document(Node root) : root(move(root)) {
  }

  const Node& Document::GetRoot() const {
    return root;
  }

  Node LoadNode(istream& input);

  Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']'; ) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadBool(istream& input) {
      bool result = false;
      if (input.peek() == 'f') {
          input.ignore(5);
      } else {
          result = true;
          input.ignore(4);
      }
      return result;
  }

  Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
      result *= 10;
      result += input.get() - '0';
    }
    return Node(result);
  }

  Node LoadNumber(istream& input) {
      bool is_positive = true;
      if (input.peek() == '-') {
          is_positive = false;
          input.ignore();
      }
      int result = LoadInt(input).AsInt();
      if (input.peek() == '.') {
          input.ignore();
          double result_double = result;
          double append_power = 0.1;
          while (isdigit(input.peek())) {
              result_double += append_power * (input.get() - '0');
              append_power /= 10;
          }
          return is_positive ? result_double : -1 * result_double;
      }
      return is_positive ? result : -1 * result;
  }

  Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream& input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}'; ) {
      if (c == ',') {
        input >> c;
      }

      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }

    return Node(move(result));
  }

    Node LoadNode(istream& input) {
        char c;
        while (input.peek() == ' ') input.ignore(1);
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if (isdigit(c) || c == '-') {
            input.putback(c);
            return LoadNumber(input);
        } else if (c == 't' || c == 'f') {
            input.putback(c);
            return LoadBool(input);
        }
        throw std::runtime_error("unknown type of c");
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document& document, std::ostream& output) {
        output << "[\n";
        const vector<Node>& responses = document.GetRoot().AsArray();
        for (auto response_map = responses.cbegin(); response_map != responses.cend(); ++response_map) {
            output << "{\n";
            const map<string, Node> response_params = response_map->AsMap();
            for (auto response_param = response_params.cbegin(); response_param != response_params.cend(); ++response_param) {
                output << "\"" << response_param->first << "\": " << response_param->second;
                if (next(response_param) != response_params.cend()) output << ',';
                output << '\n';
            }
            output << '}';
            if (next(response_map) != responses.cend()) output << ',';
            output << '\n';
        }
        output << "]";
    }
}
