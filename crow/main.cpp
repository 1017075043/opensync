#include "crow.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/filesystem.hpp>

#include <unordered_set>
#include <mutex>

using namespace std;

void getView(crow::response& res, const std::string& filename, crow::mustache::context& x)
{
    res.set_header("Content-Type", "text/html");
    auto text = crow::mustache::load("html//" + filename + ".html").render(x);
    res.write(text);
    res.end();
}

void sendFile(crow::response& res, std::string filename, std::string contentType)
{
    std::ifstream in("/home/wnh/projects/crow/site/" + filename, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res.set_header("Content-Type", contentType);
        res.write(contents.str());
    }
    else
    {
        res.code = 404;
        res.write("Not found");
    }
    res.end();
}

void sendHtml(crow::response& res, std::string filename)
{
    sendFile(res, "html/" + filename, "text/html");
}

void sendImage(crow::response& res, std::string filename)
{
    sendFile(res, "images/" + filename, "image/jpeg");
}

void sendScript(crow::response& res, std::string filename)
{
    sendFile(res, "scripts/" + filename, "text/javascript");
}

void sendStyle(crow::response& res, std::string filename)
{
    sendFile(res, "styles/" + filename, "text/css");
}

void notFound(crow::response& res, const std::string& message)
{
    res.code = 404;
    res.write(message + ": Not Found");
    res.end();
}

int main(int argc, char* argv[])
{
    crow::SimpleApp app;

    //CROW_ROUTE(app, "/html/<string>")
    //    ([](const crow::request& req, crow::response& res, std::string filename) {
    //    sendHtml(res, filename);
    //        });
    crow::mustache::set_base("/home/wnh/projects/crow/site/");
    CROW_ROUTE(app, "/html/b.html")
        ([](const crow::request& req, crow::response& res) {
        //crow::mustache::context ctx;
        //ctx["animals"][0]["name"] = "chouette";
        //ctx["animals"][0]["image"] = "owl.jpg";
        //ctx["animals"][1]["name"] = "owl";
        //ctx["animals"][1]["image"] = "owl.jpg";
        //return crow::mustache::load("//home//wnh//projects//crow//site//html//a.html").render(ctx);

        crow::mustache::context ctx;
        ctx["myquery"] = "ssssssssddddddddddddddddddssssssss";
        //
        //auto page = crow::mustache::load("//home//wnh//projects//crow//site//html//b.html");
        //return page.render();
        //
        //crow::mustache::context ctx;
        //ctx.keys();
        //return crow::mustache::load("html//b.html").render(x);

        getView(res, "b", ctx);
            });
    CROW_ROUTE(app, "/html/a.html")
        ([] {
        return crow::mustache::load("html//a.html").render();
            });

    CROW_ROUTE(app, "/styles/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename) {
        sendStyle(res, filename);
            });

    CROW_ROUTE(app, "/scripts/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename) {
        sendScript(res, filename);
            });

    CROW_ROUTE(app, "/images/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename) {
        sendImage(res, filename);
            });

    CROW_ROUTE(app, "/json")
        ([] {
        crow::json::wvalue x;
        x["message"] = "Hello, World!";
        return x;
            });

    char* port = getenv("PORT");
    uint16_t iPort = static_cast<uint16_t>(port != NULL ? std::stoi(port) : 18080);
    std::cout << "PORT = " << iPort << "\n";

    app.port(iPort).multithreaded().run();
}

/*
vector<string> msgs;
vector<pair<crow::response*, decltype(chrono::steady_clock::now())>> ress;

void broadcast(const string& msg)
{
    msgs.push_back(msg);
    crow::json::wvalue x;
    x["msgs"][0] = msgs.back();
    x["last"] = msgs.size();
    string body = crow::json::dump(x);
    for (auto p : ress)
    {
        auto* res = p.first;
        CROW_LOG_DEBUG << res << " replied: " << body;
        res->end(body);
    }
    ress.clear();
}

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello worlddddddddddddddd";
        });
    CROW_ROUTE(app, "/json")
        ([] {
        crow::json::wvalue x;
        x["message"] = "Hello, World!";
        return x;
            });
    CROW_ROUTE(app, "/hello/<int>")
        ([](int count) {
        if (count > 100)
            return crow::response(400);
        std::ostringstream os;
        os << count << " bottles of beer!";
        return crow::response(os.str());
            });
    CROW_ROUTE(app, "/params")
        ([](const crow::request& req) {
        std::ostringstream os;
        os << "Params: " << req.url_params << "\n\n";
        os << "The key 'foo' was " << (req.url_params.get("foo") == nullptr ? "not " : "") << "found.\n";
        if (req.url_params.get("pew") != nullptr) {
            double countD = boost::lexical_cast<double>(req.url_params.get("pew"));
            os << "The value of 'pew' is " << countD << '\n';
        }
        auto count = req.url_params.get_list("count");
        os << "The key 'count' contains " << count.size() << " value(s).\n";
        for (const auto& countVal : count) {
            os << " - " << countVal << '\n';
        }
        return crow::response{ os.str() };
            });
    CROW_ROUTE(app, "/add_json")
        .methods("POST"_method)
        ([](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x)
            return crow::response(400);
        int sum = x["a"].i() + x["b"].i();
        std::ostringstream os;
        os << sum;
        return crow::response{ os.str() };
            });
    CROW_ROUTE(app, "/header")
        ([](const crow::request& req) {
        std::ostringstream os;
        // ci_map req.header
        os << "Headers Server: " << req.get_header_value("test") << "\n\n";
        return req.get_header_value("test");
            });
    crow::mustache::set_base("/home/wnh/projects/crow/site/");
    CROW_ROUTE(app, "/html/example_chat.html")
        ([] {
        crow::mustache::context ctx;
        ctx.keys();
        return crow::mustache::load("html//example_chat.html").render();
          });
    CROW_ROUTE(app, "/images/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename) {
        std::ifstream in("/home/wnh/projects/crow/site/images/a.jpg", std::ifstream::in);
        if (in)
        {
            std::ostringstream contents;
            contents << in.rdbuf();
            in.close();
            res.set_header("Content-Type", "image/jpeg");
            res.write(contents.str());
        }
        else
        {
            res.code = 404;
            res.write("Not found");
        }
        res.end();
            });
    //crow::mustache::set_base("/home/wnh/projects/crow/site/images/");
    //CROW_ROUTE(app, "/images/a.jpg")
    //    ([] {
    //    crow::mustache::context ctx;
    //    return crow::mustache::load("images//a.jpg").render();
    //        });
    //CROW_ROUTE(app, "/logs")
    //    ([] {
    //    CROW_LOG_INFO << "logs requested";
    //    crow::json::wvalue x;
    //    int start = max(0, (int)msgs.size() - 100);
    //    for (int i = start; i < (int)msgs.size(); i++)
    //        x["msgs"][i - start] = msgs[i];
    //    x["last"] = msgs.size();
    //    CROW_LOG_INFO << "logs completed";
    //    return x;
    //        });
    CROW_ROUTE(app, "/send")
        .methods("GET"_method, "POST"_method)
        ([](const crow::request& req)
            {
                CROW_LOG_INFO << "msg from client: " << req.body;
                broadcast(req.body);
                return "";
            });
    CROW_ROUTE(app, "/logs/<int>")
        ([](const crow::request&, crow::response& res, int after) {
        CROW_LOG_INFO << "logs with last " << after;
        if (after < (int)msgs.size())
        {
            crow::json::wvalue x;
            for (int i = after; i < (int)msgs.size(); i++)
                x["msgs"][i - after] = msgs[i];
            x["last"] = msgs.size();

            res.write(crow::json::dump(x));
            res.end();
        }
        else
        {
            vector<pair<crow::response*, decltype(chrono::steady_clock::now())>> filtered;
            for (auto p : ress)
            {
                if (p.first->is_alive() && chrono::steady_clock::now() - p.second < chrono::seconds(30))
                    filtered.push_back(p);
                else
                    p.first->end();
            }
            ress.swap(filtered);
            ress.push_back({ &res, chrono::steady_clock::now() });
            CROW_LOG_DEBUG << &res << " stored " << ress.size();
        }
            });
    app.port(18080).multithreaded().run();
}
*/