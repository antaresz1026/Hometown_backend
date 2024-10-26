/**
 * @file postManage.hpp
 * @author antaresz (antaresz1026@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <string>
#include <vector>
#include <optional>
#include "SQLConnection.hpp"

struct Post {
    int postid;                         //postid
    int upid;                           //upid
    std::string title;                  //标题
    std::string content;                //内容
    std::string post_type;              //post类型
    std::string created_at;             //创建时间
};

class postManage {
public:
    postManage(SQLConnection& connectionPool);

    bool createPost(int upid, const std::string& title, const std::string& conten, const std::string& post_typet);
    bool deletePost(int id);
    bool updatePost(int id, const std::string& title, const std::string& content);
    std::optional<Post> getPost(int id);
    std::vector<Post> getAllPosts();
private:
    SQLConnection& _connection_pool;
};
