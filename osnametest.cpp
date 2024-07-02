#include <iostream>
#include <fstream>
#include <string>

// ラズパイOS情報取得を取得する関数
void getRaspiOSInformation(std::ifstream& file,std::string& line,std::string& name, std::string& version){
        while (getline(file, line)) {
            if ((line.find("NAME") != std::string::npos) && (line.find("PRETTY") == std::string::npos) && (line.find("CODE") == std::string::npos)) {
                name = line;
            }
            if (line.find("VERSION=") != std::string::npos) {
                version=line;
            }
        }
}

// ダブルクォーテーションの中身を取り出す関数
std::string extractQuotedString(const std::string& input) {
    // 最初のダブルクオーテーションの位置を見つける
    size_t startPos = input.find('"');
    if (startPos == std::string::npos) {
        return ""; // ダブルクオーテーションが見つからない場合
    }

    // 最初のダブルクオーテーションの次の位置から検索を開始
    size_t endPos = input.find('"', startPos + 1);
    if (endPos == std::string::npos) {
        return ""; // 閉じるダブルクオーテーションが見つからない場合
    }

    // ダブルクオーテーション内の文字列を抽出
    return input.substr(startPos + 1, endPos - startPos - 1);
}


int main() {
    std::ifstream file("/etc/os-release");
    std::string line;
    std::string name;
    std::string version;

    // if (file.is_open()) {
    //     while (getline(file, line)) {
    //         if ((line.find("NAME") != std::string::npos) && (line.find("PRETTY") == std::string::npos) && (line.find("CODE") == std::string::npos)) {
    //             name = line;
    //             std::cout << name << std::endl;
    //          }
    //         if (line.find("VERSION=") != std::string::npos) {
    //             version = line;
    //             std::cout << version << std::endl;
    //         }
    //     }
    //     file.close();
    // } else {
    //     std::cerr << "Unable to open the file" << std::endl;
    //     return 1;  // エラー終了
    // }

    // ラズパイOS情報取得
    if (file.is_open()) {
        getRaspiOSInformation(file, line, name, version);
    } else {
        std::cerr << "Unable to open the file" << std::endl;
    }

    name = extractQuotedString(name);
    version = extractQuotedString(version);
    
    std::cout << name << std::endl;
    std::cout << version << std::endl;

    return 0;  // 正常終了
}