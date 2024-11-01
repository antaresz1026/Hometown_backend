#include "postManage.hpp"
#include "logger.hpp"
#include <cppconn/prepared_statement.h>

/**
 * @brief 构造函数，接收连接池的引用
 */
postManage::postManage(SQLConnection& connectionPool) : _connection_pool(connectionPool) {}

/**
 * @brief 创建新帖子
 */
bool postManage::createPost(int upid, const std::string& title, const std::string& content, const std::string& post_type) {
    auto conn = _connection_pool.getConnection();
    try {
        std::shared_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("INSERT INTO posts (upid, title, content, post_type) VALUES (?, ?, ?, ?)")
        );
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT , password FROM users WHERE username = ?")
        );
        stmt->setInt(1, upid);
        stmt->setString(2, title);
        stmt->setString(3, content);
        stmt->setString(4, post_type);
        stmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        logger::getInstance().log("error", "Failed to create post: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief 删除帖子
 */
bool postManage::deletePost(int id) {
    auto conn = _connection_pool.getConnection();
    try {
        std::shared_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("DELETE FROM posts WHERE id = ?")
        );
        stmt->setInt(1, id);
        stmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        logger::getInstance().log("error", "Failed to delete post: " + std::string(e.what()));
        return false;
    }
}

// /**
//  * @brief 获取单个帖子
//  */
// std::optional<Post> postManage::getPost(int id) {
//     auto conn = _connection_pool.getConnection();
//     try {
//         std::shared_ptr<sql::PreparedStatement> stmt(
//             conn->prepareStatement("SELECT * FROM posts WHERE id = ?")
//         );
//         stmt->setInt(1, id);
//         std::shared_ptr<sql::ResultSet> res(stmt->executeQuery());

//         if (res->next()) {
//             Post post = {
//                 res->getInt("id"),
//                 res->getString("title"),
//                 res->getString("content"),
//                 res->getString("author"),
//                 res->getString("created_at")
//             };
//             return post;
//         }
//     } catch (sql::SQLException& e) {
//         BOOST_LOG_TRIVIAL(error) << "Failed to get post: " << e.what();
//     }
//     return std::nullopt;
// }

/**
 * @brief 获取所有帖子
 */
std::vector<Post> postManage::getAllPosts() {
    std::vector<Post> posts;
    auto conn = _connection_pool.getConnection();
    try {
        std::shared_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT * FROM posts")
        );
        std::shared_ptr<sql::ResultSet> res(stmt->executeQuery());

        while (res->next()) {
            posts.emplace_back(Post{
                res->getInt("id"),
                res->getInt("upid"),
                res->getString("title"),
                res->getString("content"),
                res->getString("post_type"),
                res->getString("created_at")
            });
        }
    } catch (sql::SQLException& e) {
        logger::getInstance().log("error", "Failed to get all posts: " + std::string(e.what()));
    }
    return posts;
}

/**
 * @brief 更新帖子
 */
bool postManage::updatePost(int id, const std::string& title, const std::string& content) {
    auto conn = _connection_pool.getConnection();
    try {
        std::shared_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("UPDATE posts SET title = ?, content = ? WHERE id = ?")
        );
        stmt->setString(1, title);
        stmt->setString(2, content);
        stmt->setInt(3, id);
        stmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        logger::getInstance().log("error", "to update post: " + std::string(e.what()));
        return false;
    }
}
