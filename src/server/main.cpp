// FinTrack Pro API server
// Exposes the core library as a small JSON API and optionally serves the built dashboard.
//
// usage: fintrack_server [--data DIR] [--port N] [--static DIR]

#include <iostream>
#include <stdexcept>
#include <string>

#include <httplib.h>
#include <json.hpp>

#include "Store.h"

using nlohmann::json;

static void sendJson(httplib::Response& res, const json& body, int status = 200) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

// run a handler and turn validation / parse failures into 400 responses
template <typename Fn>
static httplib::Server::Handler guarded(Fn fn) {
    return [fn](const httplib::Request& req, httplib::Response& res) {
        try {
            fn(req, res);
        } catch (const json::exception&) {
            sendJson(res, {{"error", "Request body is missing a field or is not valid JSON"}}, 400);
        } catch (const std::invalid_argument& ex) {
            sendJson(res, {{"error", ex.what()}}, 400);
        }
    };
}

int main(int argc, char* argv[]) {
    std::string dataDir = "data";
    std::string staticDir = "web/dist";
    int port = 8080;
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string flag = argv[i];
        if (flag == "--data") dataDir = argv[i + 1];
        else if (flag == "--port") port = std::stoi(argv[i + 1]);
        else if (flag == "--static") staticDir = argv[i + 1];
    }

    Store store(dataDir);
    store.load();

    httplib::Server server;

    server.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        sendJson(res, {{"status", "ok"}});
    });
    server.Get("/api/accounts", [&](const httplib::Request&, httplib::Response& res) {
        sendJson(res, store.accountsJson());
    });
    server.Get("/api/transactions", [&](const httplib::Request&, httplib::Response& res) {
        sendJson(res, store.transactionsJson());
    });
    server.Get("/api/budgets", [&](const httplib::Request&, httplib::Response& res) {
        sendJson(res, store.budgetsJson());
    });

    server.Post("/api/accounts", guarded([&](const httplib::Request& req, httplib::Response& res) {
        json body = json::parse(req.body);
        sendJson(res, store.addAccount(body.at("name"), body.at("type")), 201);
    }));
    server.Post("/api/transactions", guarded([&](const httplib::Request& req, httplib::Response& res) {
        json body = json::parse(req.body);
        sendJson(res, store.addTransaction(body.at("account"), body.at("date"), body.at("amount"),
                                           body.at("category"), body.value("note", "")), 201);
    }));
    server.Post("/api/transfers", guarded([&](const httplib::Request& req, httplib::Response& res) {
        json body = json::parse(req.body);
        store.transfer(body.at("from"), body.at("to"), body.at("date"), body.at("amount"));
        sendJson(res, {{"status", "ok"}}, 201);
    }));
    server.Put("/api/budgets", guarded([&](const httplib::Request& req, httplib::Response& res) {
        json body = json::parse(req.body);
        store.setBudget(body.at("category"), body.at("limit"));
        sendJson(res, store.budgetsJson());
    }));

    if (server.set_mount_point("/", staticDir)) {
        std::cout << "Serving dashboard from " << staticDir << "\n";
    }

    std::cout << "FinTrack Pro API listening on http://localhost:" << port
              << " (data: " << dataDir << ")" << std::endl;
    server.listen("127.0.0.1", port);
    return 0;
}
