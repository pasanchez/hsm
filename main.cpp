#include <iostream>

#include <sqlite3.h>
#include <vector>
#include <cstring>
#include "simpleHttpServer/Server.h"
#include "json/json.h"

using namespace std;
using handler_t = int(void*,int,char**,char**);

Json::Value newItem(std::string name, float stock, float min_stock, float max_stock,std::string unit, sqlite3 *db){
    cout << "New Item " << name << endl;
    std::stringstream error_stream;
    Json::Value responseJson;
    auto callback = [](void* fromexec,int argc, char** argv, char** colName )->int{return 0;};
    std::string insert_item = "insert into ITEMS (NAME, STOCK, MIN_STOCK, MAX_STOCK,UNIT) values (\"" +
                              name +"\"," + std::to_string(stock) +"," + std::to_string(min_stock)+"," + std::to_string(max_stock) +
                              ",\""+unit+"\");";
    char *errorMsg;
    if(sqlite3_exec(db,insert_item.c_str(),callback, nullptr,&errorMsg)) {
        string error(errorMsg);
        if (error.find("UNIQUE constraint failed: ITEMS.NAME") == std::string::npos)  {
            error_stream << "Error creating item: " << errorMsg;
        } else {
            error_stream << "Item already exits.";
        };
        responseJson["message"] = error_stream.str();
        responseJson["code"] = -1;
    } else {
        responseJson["message"] = "Item added";
        responseJson["code"] = 0;
    }
    return responseJson;
}

Json::Value searchItem(std::string name, sqlite3 *db){
    cout << "Search Item " << name << endl;
    std::stringstream error_stream;
    Json::Value responseJson;
    auto callback = [](void* fromexec,int argc, char** argv, char** colName )->int{
        Json::Value *responseJson = (Json::Value *)fromexec;
        Json::Value thisJson;
        for (int i=0; i < argc; i++) {
            thisJson[colName[i]] =(argv[i]?argv[i]:"Null");
        }
        (*responseJson)["items"].append(thisJson);
        return 0;
    };
    std::string search = "select * from ITEMS where upper(NAME) like upper(\"%" +name+ "%\");";
    char *errorMsg;
    if(sqlite3_exec(db,search.c_str(),callback, &responseJson,&errorMsg)) {
        string error(errorMsg);
        error_stream << "Error retrieving items: " << errorMsg << endl;
        responseJson["message"] = error_stream.str();
        responseJson["code"] = -1;
    } else {
        responseJson["message"] = "OK";
        responseJson["code"] = 0;
    }
    return  responseJson;
}

Json::Value getItem(std::string ID, sqlite3 *db){
    cout << "Get item ID: " << ID << endl;
    std::stringstream error_stream;
    Json::Value responseJson;
    auto callback = [](void* fromexec,int argc, char** argv, char** colName )->int{
        Json::Value *responseJson = (Json::Value *)fromexec;
        for (int i=0; i < argc; i++) {
            (*responseJson)[colName[i]] =(argv[i]?argv[i]:"Null");
        }
        return 0;
    };
    std::string search = "select * from ITEMS where ID= " + ID + ";";
    char *errorMsg;
    if(sqlite3_exec(db,search.c_str(),callback, &responseJson,&errorMsg)) {
        string error(errorMsg);
        error_stream << "Error retrieving item: " << errorMsg;
        responseJson["message"] = error_stream.str();
        responseJson["code"] = -1;
    } else {
        responseJson["message"] = "OK";
        responseJson["code"] = 0;
    }
    return  responseJson;
}

Json::Value getShoppingList(sqlite3 *db){
    cout << "Get Shopping List" << endl;
    std::stringstream error_stream;
    Json::Value responseJson;
    std::string search = "select * from ITEMS where STOCK < MIN_STOCK;";
    char *errorMsg;
    auto handler = [](void* fromexec,int argc, char** argv, char** colName )->int{
        Json::Value *responseJson = (Json::Value *)fromexec;

        Json::Value thisJson;
        thisJson["name"] = argv[1];
        thisJson["qty"] = std::stof(argv[4]) - std::stof(argv[2]);
        thisJson["unit"] = argv[5];
        (*responseJson)["items"].append(thisJson);
        return 0;
    };
    if(sqlite3_exec(db,search.c_str(),handler, &responseJson,&errorMsg)) {
        string error(errorMsg);
        error_stream << "Error retrieving items: " << errorMsg;
        responseJson["message"] = error_stream.str();
        responseJson["code"] = -1;
    } else {
        responseJson["message"] = "OK";
        responseJson["code"] = 0;
    }
    return responseJson;
}

Json::Value deleteItem(std::string ID,  sqlite3 *db){
    std::stringstream error_stream;
    Json::Value responseJson;
    auto callback = [](void* fromexec,int argc, char** argv, char** colName )->int{return 0;};
    std::string delete_item = "delete from ITEMS where ID=" + ID + ";";
    char *errorMsg;
    if(sqlite3_exec(db,delete_item.c_str(),callback, nullptr,&errorMsg)) {
        string error(errorMsg);
        error_stream << "Error deleting item: " << errorMsg;
        responseJson["message"] = error_stream.str();
        responseJson["code"] = -1;
    } else {
        responseJson["message"] = "Item deleted";
        responseJson["code"] = 0;
    }
    return responseJson;
}

Json::Value updateStock(std::string ID, std::string add_v,  sqlite3 *db){
    cout << "Update stock of ID: " << ID << " to value: " << add_v << endl;
    std::stringstream error_stream;
    Json::Value responseJson;
    auto callback = [](void* fromexec,int argc, char** argv, char** colName )->int{return 0;};
    std::string add_stock = "update ITEMS set STOCK = "+ add_v + " where ID=" + ID + ";";
    char *errorMsg;
    if(sqlite3_exec(db,add_stock.c_str(),callback, nullptr,&errorMsg)) {
        string error(errorMsg);
        error_stream << "Error changing stock: " << errorMsg;
        responseJson["message"] = error_stream.str();
        responseJson["code"] = -1;
    } else {
        responseJson = getItem(ID,db);
        responseJson["message"] = "Item modified.";
        responseJson["code"] = 0;
    }
    return responseJson;
}

int main(int argc, char* argv[]) {

    sqlite3 *db;

    if (sqlite3_open("hsm.db",&db)) {
        cout << "Error opening db." << endl;
        return -1;
    }

    std::string create_items_table = "create table if not exists ITEMS ("\
        "ID integer primary key autoincrement,"\
        "NAME text unique,"\
        "STOCK float,"\
        "MIN_STOCK float,"\
        "MAX_STOCK float,"\
        "UNIT text);";

    char *errorMsg;
    auto callback = [](void* fromexec,int argc, char** argv, char** colName )->int{ return 0; };

    if(sqlite3_exec(db,create_items_table.c_str(),callback, nullptr,&errorMsg)) {
        cout << "Error creating item table: " << errorMsg << endl;
        return -1;
    }

    Server server(8080);
    cout << "Listening on port 8080." << endl;
    server.registerAction("/n",[db](std::unordered_map<std::string,std::string> params)->std::string {
        std::string response;
        response = "HTTP/1.0 200 OK\r\n Content-Length: 45\r\n\r\n";
        response += "<html><body>";
        if (params.find("name") != params.end()
            && params.find("stock") != params.end()
               && params.find("min_stock") != params.end()
                  && params.find("max_stock") != params.end()
                     && params.find("unit") != params.end()) {

            stringstream stream;
            stream << newItem(params["name"],std::stof(params["stock"]),
                                std::stof(params["min_stock"]),std::stof(params["max_stock"]),params["unit"],db);
            response += stream.str();
        } else {
            response += "<b><big>Bad syntax. Usage: /?name=item_name&stock=item_stock&min_stock"\
                    "=item_min&max_stock=item_max&unit=item_unit</big></b> <br>";
        }
        response += "</body></html>";
        cout << "Sending response." << endl;
        return  response;
    });
    server.registerAction("/s",[db](std::unordered_map<std::string,std::string> params)->std::string {
        std::string response;
        response = "HTTP/1.0 200 OK\r\n Content-Length: 45\r\n\r\n";
        response += "<html><body>";
        if (params.find("name") != params.end()){
            stringstream stream;
            stream << searchItem(params["name"],db);
            response += stream.str();
        } else {
            response += "<b><big>Bad syntax. Usage: /?name=item_name</big></b> <br>";
        }
        response += "</body></html>";
        return  response;
    });
    server.registerAction("/list",[db](std::unordered_map<std::string,std::string> params)->std::string {
        std::string response;
        response = "HTTP/1.0 200 OK\r\n Content-Length: 45\r\n\r\n";
        response += "<html><body>";
        stringstream stream;
        stream << getShoppingList(db);
        response += stream.str();
        response += "</body></html>";
        return  response;
    });

    server.registerAction("/d",[db](std::unordered_map<std::string,std::string> params)->std::string {
        std::string response;
        response = "HTTP/1.0 200 OK\r\n Content-Length: 45\r\n\r\n";
        response += "<html><body>";
        if (params.find("ID") != params.end()){
            stringstream stream;
            stream <<  deleteItem(params["ID"],db);
            response += stream.str();
        } else {
            response += "<b><big>Bad syntax. Usage: /?name=item_name</big></b> <br>";
        }
        response += "</body></html>";
        return  response;
    });
    server.registerAction("/u",[db](std::unordered_map<std::string,std::string> params)->std::string {
        std::string response;
        response = "HTTP/1.0 200 OK\r\n Content-Length: 45\r\n\r\n";
        response += "<html><body>";
        if (params.find("ID") != params.end() && params.find("val") != params.end()){
            stringstream stream;
            stream <<  updateStock(params["ID"],params["val"],db);
            response += stream.str();
        } else {
            response += "<b><big>Bad syntax. Usage: /?name=item_name</big></b> <br>";
        }
        response += "</body></html>";
        return  response;
    });


    server.listen();
    return 0;
}


