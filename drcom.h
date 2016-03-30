/*
 *  JLU Drcom Gtk - Client to login to Jilin University network
 *  Copyright (C) 2016  Jonathan Kang
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRCOM_H_
#define DRCOM_H_

#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>

// 必须修改，帐号密码和 mac 地址是绑定的
static uint64_t mac = 0x000000000000; // echo 0x`ifconfig eth | egrep -io "([0-9a-f]{2}:){5}[0-9a-f]{2}" | tr -d ":"`

// 不一定要修改
static char host[] = "drcom";
static char os[] = "drcom";
static int host_len = sizeof(host) - 1;
static int os_len = sizeof(os) - 1;

/* signal process flag */
static int logout_flag = 0;

// TODO 增加从文件读取参数

//SERVER_DOMAIN login.jlu.edu.cn
#define SERVER_ADDR "10.100.61.3"
#define SERVER_PORT 61440

#define RECV_DATA_SIZE 1000
#define SEND_DATA_SIZE 1000
#define CHALLENGE_TRY 10
#define LOGIN_TRY 5
#define ALIVE_TRY 5

/* infomation */
struct user_info_pkt {
	const char *username;
	const char *password;
	char *hostname;
	char *os_name;
	uint64_t mac_addr;
	int username_len;
	int password_len;
	int hostname_len;
	int os_name_len;
};

void get_user_info (struct user_info_pkt *user_info,
                    const char *username,
                    const char *password);
void set_challenge_data (unsigned char *clg_data,
                         int clg_data_len,
                         int clg_try_count);
bool challenge (int sock,
                struct sockaddr_in serv_addr,
                unsigned char *clg_data,
                int clg_data_len,
                char *recv_data,
                int recv_len);
void set_login_data (struct user_info_pkt *user_info,
                     unsigned char *login_data,
                     int login_data_len,
                     unsigned char *salt,
                     int salt_len);
bool login (int sock,
            struct sockaddr_in serv_addr,
            unsigned char *login_data,
            int login_data_len,
            char *recv_data,
            int recv_len);
void set_alive_data (unsigned char *alive_data,
                     int alive_data_len,
                     unsigned char *tail,
                     int tail_len,
                     int alive_count,
                     int random);
void set_logout_data (unsigned char *logout_data,
                      int logout_data_len);
bool logout (int sock,
             struct sockaddr_in serv_addr,
             unsigned char *logout_data,
             int logout_data_len,
             char *recv_data,
             int recv_len);
void logout_signal (int signum);
void on_login (const char *username, const char *password);

#endif /* DRCOM_H_ */
