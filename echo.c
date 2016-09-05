/*
 * $Id: echo-server-1.c,v 1.6 2005/02/19 16:01:53 68user Exp $
 *
 * echo サーバサンプル
 *
 * written by 68user  http://X68000.q-e-d.net/~68user/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_LEN 256             /* バッファのサイズ */

/* ソケット socket から1行読み込み、読み込んだ文字列を p に格納する。
   改行コードを読み込む前にソケットから read できなくなった場合は、
   その時点で呼び出し元に戻る。

   戻り値で読み込んだ文字数を返す。p は \0 でターミネートする。
   */
int read_line(int socket, char *p){
    int len = 0;
    while (1){
        int ret;
        ret = read(socket, p, 1);
        if ( ret == -1 ){
            perror("read");
            exit(1);
        } else if ( ret == 0 ){
            break;
        }
        if ( *p == '\n' ){
            p++;
            len++;
            break;
        }
        p++;
        len++;
    }
    *p = '\0';
    return len;
}


int main(int argc, char *argv[]){
    int connected_socket, listening_socket;
    struct sockaddr_in sin;
    int len, ret;
    int sock_optval = 1;
    int port = 5000;
    /* リスニングソケットを作成 */
    listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( listening_socket == -1 ){
        perror("socket");
        exit(1);
    }
    /* ソケットオプション設定 */
    if ( setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,
                &sock_optval, sizeof(sock_optval)) == -1 ){
        perror("setsockopt");
        exit(1);
    }
    /* アドレスファミリ・ポート番号・IPアドレス設定 */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    /* ソケットにアドレス(＝名前)を割り付ける */
    if ( bind(listening_socket, (struct sockaddr *)&sin, sizeof(sin)) < 0 ){
        perror("bind");
        exit(1);
    }
    /* ポートを見張るよう、OS に命令する */
    ret = listen(listening_socket, SOMAXCONN);
    if ( ret == -1 ){
        perror("listen");
        exit(1);
    }
    printf("ポート %d を見張ります。\n", port);

    while (1){
        struct hostent *peer_host;
        struct sockaddr_in peer_sin;

        len = sizeof(peer_sin);
        /* コネクション受け付け */
        connected_socket = accept(listening_socket, (struct sockaddr *)&peer_sin, &len);
        if ( connected_socket == -1 ){
            perror("accept");
            exit(1);
        }
        /* 相手側のホスト・ポート情報を表示 */
        peer_host = gethostbyaddr((char *)&peer_sin.sin_addr.s_addr,
                sizeof(peer_sin.sin_addr), AF_INET);
        if ( peer_host == NULL ){
            printf("gethostbyname failed\n");
            exit(1);
        }

        printf("接続: %s [%s] ポート %d\n",
                peer_host->h_name,
                inet_ntoa(peer_sin.sin_addr),
                ntohs(peer_sin.sin_port)
              );

        while (1){
            int read_size;
            char buf[BUF_LEN];
            /* 1行読み込む */
            read_size = read_line(connected_socket, buf);
            if ( read_size == 0 ) break;

            printf("メッセージ: %s", buf);
            /* クライアントに文字列をそのまま返す */
            write(connected_socket, buf, strlen(buf));
        }

        printf("接続が切れました。引き続きポート %d を見張ります。\n", port);
        ret = close(connected_socket);
        if ( ret == -1 ){
            perror("close");
            exit(1);
        }
    }
    ret = close(listening_socket);
    if ( ret == -1 ){
        perror("close");
        exit(1);
    }

    return 0;
}
