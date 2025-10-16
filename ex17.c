#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

// 单条记录
struct Address {
    int id;
    int set;  // 标记是否已设置（1 表示已设置，0 表示空）
    char *name;
    char *email;
};

// 数据库
struct Database {
    int max_data;  // 每个字符串的最大长度
    int max_rows;  // 最多记录数
    struct Address *rows;  // 指向记录数组的指针
};

// 数据库链接
struct Connection {
    FILE *file;
    struct Database *db;
};

// 全局数据库连接
static struct Connection *g_conn = NULL;

// 关闭数据库连接
void Database_close()
{
    if(g_conn) {
        if(g_conn->file) fclose(g_conn->file);  // 关闭文件
        if(g_conn->db) {
			// 释放每一条记录的姓名和邮箱字符串内存
            for(int i = 0; i < g_conn->db->max_rows; i++) {
                free(g_conn->db->rows[i].name);
                free(g_conn->db->rows[i].email);
            }
            free(g_conn->db->rows);  // 释放记录数组
            free(g_conn->db);  	     // 释放数据库
        }
        free(g_conn);  // 释放链接结构体
        g_conn = NULL;
    }
}

// 错误处理
void die(const char *message)
{
    if(errno) {  // 如果 errno 被设置，则用 perror 输出系统错误信息
        perror(message);
    } else {     // 否则输出自定义错误信息
        printf("ERROR: %s\n", message);
    }
    Database_close();
    exit(1);  // 非正常退出
}

// 打印单条记录
void Address_print(struct Address *addr)
{
    printf("%d %s %s\n", addr->id, addr->name, addr->email);
}

// 从文件加载数据库到内存
void Database_load()
{
    int max_data, max_rows;
    // 读取字符串最大长度和最多记录数量
    fread(&max_data, sizeof(int), 1, g_conn->file);
    fread(&max_rows, sizeof(int), 1, g_conn->file);

    g_conn->db->max_data = max_data;
    g_conn->db->max_rows = max_rows;
	
	// 创建记录数组
    g_conn->db->rows = malloc(sizeof(struct Address) * max_rows);
    if(!g_conn->db->rows) die("Memory error");
	
	// 逐条读取数据
    for(int i = 0; i < max_rows; i++) {
        struct Address *addr = &g_conn->db->rows[i];
        addr->name = malloc(max_data);
        addr->email = malloc(max_data);

        fread(&addr->id, sizeof(int), 1, g_conn->file);
        fread(&addr->set, sizeof(int), 1, g_conn->file);
        fread(addr->name, max_data, 1, g_conn->file);
        fread(addr->email, max_data, 1, g_conn->file);
    }
}

// 打开数据库文件
void Database_open(const char *filename, char mode)
{
    g_conn = malloc(sizeof(struct Connection));
    if(!g_conn) die("Memory error");

    g_conn->db = malloc(sizeof(struct Database));
    if(!g_conn->db) die("Memory error");

    if(mode == 'c') {
		// 创建模式：新建（覆盖）文件
        g_conn->file = fopen(filename, "w");
    } else {
		// 其它模式：读取并更新文件
        g_conn->file = fopen(filename, "r+");
        if(g_conn->file) {
            Database_load();
        }
    }

    if(!g_conn->file) die("Failed to open the file");
}

// 将数据库写入文件
void Database_write()
{
    rewind(g_conn->file);  // 回到文件开头
	
	// 写入数据库大小
    fwrite(&g_conn->db->max_data, sizeof(int), 1, g_conn->file);
    fwrite(&g_conn->db->max_rows, sizeof(int), 1, g_conn->file);
	
	// 写入记录数据
    for(int i = 0; i < g_conn->db->max_rows; i++) {
        struct Address *addr = &g_conn->db->rows[i];
        fwrite(&addr->id, sizeof(int), 1, g_conn->file);
        fwrite(&addr->set, sizeof(int), 1, g_conn->file);
        fwrite(addr->name, g_conn->db->max_data, 1, g_conn->file);
        fwrite(addr->email, g_conn->db->max_data, 1, g_conn->file);
    }

    fflush(g_conn->file);  // 刷新缓冲区到磁盘
}

// 创建新的数据库（初始化所有记录为空）
void Database_create(int max_data, int max_rows)
{
    g_conn->db->max_data = max_data;
    g_conn->db->max_rows = max_rows;

    g_conn->db->rows = malloc(sizeof(struct Address) * max_rows);
    if(!g_conn->db->rows) die("Memory error");

    for(int i = 0; i < max_rows; i++) {
        g_conn->db->rows[i].id = i;
        g_conn->db->rows[i].set = 0;
        g_conn->db->rows[i].name = calloc(1, max_data);
        g_conn->db->rows[i].email = calloc(1, max_data);
    }
}

// 添加一条记录
void Database_set(int id, const char *name, const char *email)
{
    struct Address *addr = &g_conn->db->rows[id];
    if(addr->set) die("Already set, delete it first");   // 如果已存在则报错

    addr->set = 1;
    strncpy(addr->name, name, g_conn->db->max_data);
    strncpy(addr->email, email, g_conn->db->max_data);
}

// 获取一条记录
void Database_get(int id)
{
    struct Address *addr = &g_conn->db->rows[id];
    if(addr->set) {
        Address_print(addr);
    } else {
        die("ID is not set");
    }
}

// 删除一条记录
void Database_delete(int id)
{
    struct Address addr = {.id = id, .set = 0};
    free(g_conn->db->rows[id].name);
    free(g_conn->db->rows[id].email);
    g_conn->db->rows[id] = addr;
    addr.name = calloc(1, g_conn->db->max_data);
    addr.email = calloc(1, g_conn->db->max_data);
    g_conn->db->rows[id] = addr;
}

// 列出所有已设置的记录
void Database_list()
{
    for(int i = 0; i < g_conn->db->max_rows; i++) {
        struct Address *cur = &g_conn->db->rows[i];
        if(cur->set) {
            Address_print(cur);
        }
    }
}

// 查找记录（按照关键字匹配姓名或邮箱）
void Database_find(const char *keyword)
{
    int found = 0;
    for(int i = 0; i < g_conn->db->max_rows; i++) {
        struct Address *cur = &g_conn->db->rows[i];
        if(cur->set) {
			// 如果关键字在姓名或邮箱中出现
            if(strstr(cur->name, keyword) || strstr(cur->email, keyword)) {
                Address_print(cur);
                found = 1;
            }
        }
    }
    if(!found) {
        printf("No match found for '%s'\n", keyword);
    }
}


// 程序入口
int main(int argc, char *argv[])
{
    if(argc < 3) die("USAGE: ex17 <dbfile> <action> [action params]");

    char *filename = argv[1];
    char action = argv[2][0];
    Database_open(filename, action);

    int id = 0;

    switch(action) {
        case 'c':  // 创建数据库
            if(argc != 5) die("Need max_data and max_rows");
            Database_create(atoi(argv[3]), atoi(argv[4]));
            Database_write();
            break;

        case 'g':  // 获取记录
            if(argc != 4) die("Need an id to get");
            id = atoi(argv[3]);
            if(id >= g_conn->db->max_rows) die("There's not that many records.");
            Database_get(id);
            break;

        case 's':  // 设置记录
            if(argc != 6) die("Need id, name, email to set");
            id = atoi(argv[3]);
            if(id >= g_conn->db->max_rows) die("There's not that many records.");
            Database_set(id, argv[4], argv[5]);
            Database_write();
            break;

        case 'd':  // 删除记录
            if(argc != 4) die("Need id to delete");
            id = atoi(argv[3]);
            if(id >= g_conn->db->max_rows) die("There's not that many records.");
            Database_delete(id);
            Database_write();
            break;

        case 'l':  // 列出所有记录
            Database_list();
            break;
		
		case 'f':  // 查找记录
			if(argc != 4) die("Need a keyword to find");
			Database_find(argv[3]);
			break;

        default:  // 无效操作
            die("Invalid action, only: c=create, g=get, s=set, d=del, l=list");
    }

    Database_close();
    return 0;
}
