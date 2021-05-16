#pragma once

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <iostream>

namespace Json {

  class Node : std::variant<std::vector<Node>,
                            std::map<std::string, Node>,
                            bool,
                            int,
                            double,
                            std::string> {
  public:
    using variant::variant;

    const auto& AsArray() const {
      return std::get<std::vector<Node>>(*this);
    }
    const auto& AsMap() const {
      return std::get<std::map<std::string, Node>>(*this);
    }
    bool AsBool() const {
        return std::get<bool>(*this);
    }
    int AsInt() const {
      return std::get<int>(*this);
    }
    const auto& AsString() const {
      return std::get<std::string>(*this);
    }
    double AsDouble() const {
        return std::get<double>(*this);
    };
    bool HoldsInt() const {
        return std::holds_alternative<int>(*this);
    }
    friend std::ostream& operator << (std::ostream&, const Node&);
  };

  class Document {
  public:
    explicit Document(Node root);

    const Node& GetRoot() const;

  private:
    Node root;
  };

  Document Load(std::istream& input = std::cin);
  void Print(const Document&, std::ostream& output = std::cout);
}
