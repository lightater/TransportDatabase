#include "tests.h"
#include "svg_library.h"
#include "transport_database.h"
#include "utils.h"
#include "requests.h"
#include "json.h"
#include <fstream>

using namespace std;

void TestSvg() {
    Svg::Document svg;

    svg.Add(
            Svg::Polyline{}
                    .SetStrokeColor(Svg::Rgb{140, 198, 63})  // soft green
                    .SetStrokeWidth(16)
                    .SetStrokeLineCap("round")
                    .AddPoint({50, 50})
                    .AddPoint({250, 250})
    );
    for (const auto point : {Svg::Point{50, 50}, Svg::Point{250, 250}}) {
        svg.Add(
                Svg::Circle{}
                        .SetFillColor("white")
                        .SetRadius(6)
                        .SetCenter(point)
        );
    }
    svg.Add(
            Svg::Text{}
                    .SetPoint({50, 50})
                    .SetOffset({10, -10})
                    .SetFontSize(20)
                    .SetFontFamily("Verdana")
                    .SetFillColor("black")
                    .SetData("C")
    );
    svg.Add(
            Svg::Text{}
                    .SetPoint({250, 250})
                    .SetOffset({10, -10})
                    .SetFontSize(20)
                    .SetFontFamily("Verdana")
                    .SetFillColor("black")
                    .SetData("C++")
    );
    std::ostringstream output;
    svg.Render(output);
    std::string expected = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"
                           "<polyline fill=\"none\" stroke=\"rgb(140,198,63)\" stroke-width=\"16\" stroke-linecap=\"round\" points=\"50,50 250,250 \" />"
                           "<circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" r=\"6\" cx=\"50\" cy=\"50\" />"
                           "<circle fill=\"white\" stroke=\"none\" stroke-width=\"1\" r=\"6\" cx=\"250\" cy=\"250\" />"
                           "<text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"50\" y=\"50\" dx=\"10\" dy=\"-10\" font-size=\"20\" font-family=\"Verdana\" >C</text>"
                           "<text fill=\"black\" stroke=\"none\" stroke-width=\"1\" x=\"250\" y=\"250\" dx=\"10\" dy=\"-10\" font-size=\"20\" font-family=\"Verdana\" >C++</text>"
                           "</svg>\n";
    ASSERT_EQUAL(output.str(), expected)
}

void TestReadToken() {
    string str = "hello, world\n"
                 "One, two  , three  ";
    {
        string_view sv = str;
        vector<string> result;
        while (!sv.empty()) {
            result.emplace_back(ReadToken(sv, ","));
        }
        vector<string> expected = {"hello",
                                   "world\nOne",
                                   "two",
                                   "three"};
        ASSERT_EQUAL(result, expected)
    }
    {
        string_view sv = str;
        vector<string> result;
        while (!sv.empty()) {
            result.emplace_back(ReadToken(sv, "\n"));
        }
        vector<string> expected = {"hello, world",
                                   "One, two  , three"};
        ASSERT_EQUAL(result, expected)
    }
}

bool IsEqual(double x, double y) {
    static constexpr double eps = 0.1;
    return std::abs(x - y) < eps;
}

void TestPoint() {
    using namespace Points;
    {
        Point p1 {Latitude(55.50), Longitude(37.50)};
        Point p2 {Latitude(59.50), Longitude(30.50)};
        ASSERT(IsEqual(CalcLength(p1, p2), 609903.22))
    }
    {
        Point p1 {Latitude(1), Longitude(37)};
        Point p2 {Latitude(59), Longitude(30)};
        ASSERT(IsEqual(CalcLength(p1, p2), 6478101.57))
    }
}

void TestNode() {
    using namespace Json;
    Node node = vector<Node> {"hello"s, 14, vector<Node> {"world"s}, 15.65, false, true};
    ASSERT_EQUAL(node.AsArray()[0].AsString(), "hello")
    ASSERT_EQUAL(node.AsArray()[1].AsInt(), 14)
    ASSERT_EQUAL(node.AsArray()[2].AsArray()[0].AsString(), "world")
    ASSERT_EQUAL(node.AsArray()[3].AsDouble(), 15.65)
    ASSERT_EQUAL(node.AsArray()[4].AsBool(), false)
    ASSERT_EQUAL(node.AsArray()[5].AsBool(), true)
    Node node_map = map<string, Node> {{"hello", "world"s}, {"goto", 15.4}, {"vev", vector<Node> {
        map<string, Node> {{"number", 17.18}}
    }}};
    ASSERT_EQUAL(node_map.AsMap().at("hello").AsString(), "world")
    ASSERT_EQUAL(node_map.AsMap().at("goto").AsDouble(), 15.4)
    ASSERT_EQUAL(node_map.AsMap().at("vev").AsArray()[0].AsMap().at("number").AsDouble(), 17.18)
}

void TestJson() {
    using namespace Json;
    {
        string str = "[\n"
                     "{\n"
                     "\"Route_length\": \"5950\"\n"
                     "}\n"
                     "]";
        istringstream input(str);
        ostringstream output;
        Document doc = Load(input);
        Print(doc, output);
        ASSERT_EQUAL(output.str(), str)
    }
    string input_str = "[\n"
                       "{\n"
                       "\"curvature\": 1.36124,\n"
                       "\"request_id\": 1965312327,\n"
                       "\"route_length\": 5950,\n"
                       "\"stop_count\": 6,\n"
                       "\"tmp_var\": \"13\",\n"
                       "\"unique_stop_count\": 5\n"
                       "},\n"
                       "{\n"
                       "\"curvature\": -0.31808,\n"
                       "\"request_id\": -519139350,\n"
                       "\"route_length\": -27600,\n"
                       "\"stop_count\": 0.01,\n"
                       "\"unique_stop_count\": -3\n"
                       "}\n"
                       "]";
    istringstream input(input_str);
    ostringstream output;
    Print(Load(input), output);
    ASSERT_EQUAL(output.str(), input_str)
}

void TestExample(string path_input, string path_output) {
    using namespace Transport;
    using namespace Requests;
    ifstream input(path_input);
    ostringstream output;
    TransportDatabase tdb;
    Json::Document result = ProcessRequests(ParseRequests(Json::Load(input)), tdb);
    Json::Print(result, output);
    ifstream i_expected(path_output);
    string expected;
    getline(i_expected, expected, '\0');
    ASSERT_EQUAL(output.str(), expected)
}

void TestExample1() {
    TestExample("examples/example_1.in", "examples/example_1.out");
}

void TestExample2() {
    TestExample("examples/example_2.in", "examples/example_2.out");
}

void TestExample3() {
    TestExample("examples/example_3.in", "examples/example_3.out");
}

void TestExample4() {
    TestExample("examples/transport_input4.json", "examples/transport_output4.json");
}

void TestAll() {
    TestRunner tr;
    RUN_TEST(tr, TestSvg);
    RUN_TEST(tr, TestReadToken);
    RUN_TEST(tr, TestPoint);
    RUN_TEST(tr, TestNode);
    RUN_TEST(tr, TestJson);
    RUN_TEST(tr, TestExample1);
    RUN_TEST(tr, TestExample2);
    RUN_TEST(tr, TestExample3);
}