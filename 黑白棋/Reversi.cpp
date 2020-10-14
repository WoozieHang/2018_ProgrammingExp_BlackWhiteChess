//
//  Reversi.cpp
//  ReversiClient
//
//  Created by ganjun on 2018/3/6.
//  Copyright © 2018年 ganjun. All rights reserved.
//
//newest
#include<cstdio>
#include<time.h>
#include<string>
#include<cstring>
#include "Reversi.h"
#define random(x) (rand()%x)
#define ROWS 8
#define COLS 8
#define ROUNDS 4
#define MAX_LENGTH 5
Reversi::Reversi() {
	client_socket = ClientSocket();
	oppositeColor = ownColor = -1;
	for (int row = 0; row < 8; row++)
		for (int col = 0; col < 8; col++)
			chessBoard[row][col] = 'O';
	chessBoard[3][3] = 'B';
	chessBoard[4][4] = 'B';
	chessBoard[3][4] = 'W';
	chessBoard[4][3] = 'W';
	nowStep = 0;
}

Reversi::~Reversi() {};

/*
send id and password to server by socket
rtn != 0 represents socket transfer error
*/
void Reversi::authorize(const char *id, const char *pass)
{
	client_socket.connectServer();
	std::cout << "Authorize " << id << std::endl;
	char msgBuf[BUFSIZE];
	memset(msgBuf, 0, BUFSIZE);
	msgBuf[0] = 'A';
	memcpy(&msgBuf[1], id, 9);
	memcpy(&msgBuf[10], pass, 6);
	int rtn = client_socket.sendMsg(msgBuf);
	if (rtn != 0) printf("Authorized Failed!\n");
}

// 用户id输入，服务器上需要有对应的账号密码：对应文件 players-0.txt
void Reversi::gameStart()
{
	char id[12] = { 0 }, passwd[10] = { 0 };
	//char id[12] = "111111110", passwd[10] = "123456";
	printf("ID: %s\n", id);
	gets_s(id);
	printf("PASSWD: %s\n", passwd);
	gets_s(passwd);
	authorize(id, passwd);

	printf("Game Start!\n");

	for (int round = 0; round < ROUNDS; round++) {
		roundStart(round);
		oneRound(round);
		roundOver(round);
	}
	gameOver();
	client_socket.close();
}

void Reversi::gameOver()
{
	printf("Game Over!\n");
}

// 发一次消息，走哪一步，等两个消息，1.自己的步数行不行 2. 对面走了哪一步
void Reversi::roundStart(int round)
{
	printf("Round %d Ready Start!\n", round);

	// first time receive msg from server
	int rtn = client_socket.recvMsg();
	if (rtn != 0) return;
	if (strlen(client_socket.getRecvMsg()) < 2)
		printf("Authorize Failed!\n");
	else
		printf("Round start received msg %s\n", client_socket.getRecvMsg());
	switch (client_socket.getRecvMsg()[1]) {
		// this client : black chessman
	case 'B':
		ownColor = 0;
		oppositeColor = 1;
		rtn = client_socket.sendMsg("BB");
		printf("Send BB -> rtn: %d\n", rtn);
		if (rtn != 0) return;
		break;
	case 'W':
		ownColor = 1;
		oppositeColor = 0;
		rtn = client_socket.sendMsg("BW");
		printf("Send BW -> rtn: %d\n", rtn);
		if (rtn != 0) return;
		break;
	default:
		printf("Authorized Failed!\n");
		break;
	}
}

void Reversi::oneRound(int Round)
{
	for (int row = 0; row < 8; row++)
		for (int col = 0; col < 8; col++)
			chessBoard[row][col] = 'O';
	chessBoard[3][3] = 'B';
	chessBoard[4][4] = 'B';
	chessBoard[3][4] = 'W';
	chessBoard[4][3] = 'W';
	showChessBoard(Round, 0);
	int STEP = 1;
	switch (ownColor) {
	case 0:
		while (STEP < 10000) {
			nowStep = STEP;
			pair<int, int> chess = step();                        // take action, send message

																  // lazi only excute after server's message confirm  in observe function
			generateOneStepMessage(chess.first, chess.second);


			if (observe() >= 1)
			{
				break;
			}
			showChessBoard(Round, STEP);   // receive RET Code
			saveChessBoard(Round, STEP, 0);
			if (observe() >= 1)
			{
				break;
			}
			showChessBoard(Round, STEP);// see white move
			saveChessBoard(Round, STEP, 1);
			STEP++;
			// saveChessBoard();
		}
		printf("One Round End\n");
		break;
	case 1:
		while (STEP < 10000) {

			if (observe() >= 1) break;    // see black move
			showChessBoard(Round, STEP);
			saveChessBoard(Round, STEP, 0);
			nowStep = STEP;
			pair<int, int> chess = step();                        // take action, send message
																  // lazi only excute after server's message confirm  in observe function
			generateOneStepMessage(chess.first, chess.second);


			if (observe() >= 1) break;     // receive RET Code
			showChessBoard(Round, STEP);
			saveChessBoard(Round, STEP, 1);// saveChessBoard();
			STEP++;
		}
		printf("One Round End\n");
		break;

	default:
		break;
	}
}

void Reversi::roundOver(int round)
{
	printf("Round %d Over!\n", round);
	// reset initializer

	ownColor = oppositeColor = -1;
}

int Reversi::observe()
{
	int rtn = 0;
	int recvrtn = client_socket.recvMsg();
	if (recvrtn != 0) return 1;
	printf("receive msg %s\n", client_socket.getRecvMsg());
	switch (client_socket.getRecvMsg()[0]) {
	case 'R':
	{
		switch (client_socket.getRecvMsg()[1]) {
		case 'Y':   // valid step
			switch (client_socket.getRecvMsg()[2]) {
			case 'P':   // update chessboard
			{
				int desRow = (client_socket.getRecvMsg()[3] - '0') * 10 + client_socket.getRecvMsg()[4] - '0';
				int desCol = (client_socket.getRecvMsg()[5] - '0') * 10 + client_socket.getRecvMsg()[6] - '0';
				int color = (client_socket.getRecvMsg()[7] - '0');
				//你应该在这里处理desRow和desCol，推荐使用函数
				handleMessage(desRow, desCol, color);
				printf("a valid step of : (%d %d)\n", desRow, desCol);
				break;
			}
			case 'N':   // RYN: enemy wrong step
			{
				//
				printf("a true judgement of no step\n");
				int desRow = -1;
				int desCol = -1;
				int color = (client_socket.getRecvMsg()[3] - '0');
				handleMessage(desRow, desCol, color);
				break;
			}
			}

			break;
		case 'W':
			// invalid step
			switch (client_socket.getRecvMsg()[2]) {
			case 'P': {
				int desRow = (client_socket.getRecvMsg()[3] - '0') * 10 + client_socket.getRecvMsg()[4] - '0';
				int desCol = (client_socket.getRecvMsg()[5] - '0') * 10 + client_socket.getRecvMsg()[6] - '0';

				int color = (client_socket.getRecvMsg()[7] - '0');
				printf("Invalid step , server random a true step of : (%d %d)\n", desRow, desCol);
				//你应该在这里处理desRow和desCol，推荐使用函数
				handleMessage(desRow, desCol, color);
				break;
			}
			case 'N': {
				printf("a wrong judgement of no step\n");
				int desRow = -1;
				int desCol = -1;

				int color = (client_socket.getRecvMsg()[3] - '0');
				handleMessage(desRow, desCol, color);
				break;
			}
			default:
				break;
			}
			break;
		case '1':

			printf("Error -1: Msg format error!\n");
			rtn = -1;
			break;
		case '2':

			printf("Error -2: Corrdinate error!\n");
			rtn = -2;
			break;
		case '4':

			printf("Error -4: Invalid step!\n");
			rtn = -4;
			break;
		default:

			printf("Error -5: Other error!\n");
			rtn = -5;
			break;
		}
		break;
	}
	case 'E':
	{
		switch (client_socket.getRecvMsg()[1]) {
		case '0':
			// game over
			rtn = 2;
			break;
		case '1':
			// round over
			rtn = 1;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
	return rtn;
}

void Reversi::generateOneStepMessage(int row, int col)
{
	char msg[BUFSIZE];
	memset(msg, 0, sizeof(msg));

	//put row and col in the message
	msg[0] = 'S';
	msg[1] = 'P';
	msg[2] = '0' + row / 10;
	msg[3] = '0' + row % 10;
	msg[4] = '0' + col / 10;
	msg[5] = '0' + col % 10;
	msg[6] = '\0';
	if (row<0 || col<0) row = -1, col = -1;
	if (row == -1 && col == -1) {
		msg[2] = '-';
		msg[3] = '1';
		msg[4] = '-';
		msg[5] = '1';
	}

	//print
	printf("generate one step at possition (%2d,%2d) : %s\n", row, col, msg);

	client_socket.sendMsg(msg);
}

/*-------------------------last three function--------------------------------
* step : find a good position to lazi your chess.
* saveChessBoard : save the chess board now.
* handleMessage: handle the message from server.
* 类中存在ownColor表示自己当前的棋子颜色，0表示黑棋，1表示白棋
*/
int Reversi::rtnBestPosition(int SNum)
{
	int maxro =-1;
	int maxco =-1;
	int beValuedS = 0;
	int maxValue;
	for (int ro = 0; ro<8&&beValuedS<SNum; ro++)
		for (int co = 0; co<8&& beValuedS<SNum; co++)
			if (chessBoard[ro][co] == 'S')
			{
				if (beValuedS == 0)
				{
					maxro = ro;
					maxco = co;
					maxValue = evaluePosition(ro, co, ownColor, chessBoard);
				}
				else
				{
					int value = evaluePosition(ro, co,  ownColor, chessBoard);
					if (value > maxValue)
					{
						maxValue = value;
						maxro = ro;
						maxco = co;
					}
				}
				beValuedS++;
			}
	return 10 * maxro + maxco;
}
int Reversi::evaluePosition(int ro, int co, int color, char aChessBoard[][8])
{
	//重开棋盘一次，防止破坏原有的棋盘
	char nowChessBoard[8][8];
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			nowChessBoard[i][j] = aChessBoard[i][j];
	//完成落子翻子的操作
	int opcolor = color ^ 1;
	fixAndReverseChess(ro, co, color, nowChessBoard);
	releaseP(nowChessBoard);
	upDateP(ro, co, nowChessBoard);
	releaseS(nowChessBoard);
	updateS(opcolor, nowChessBoard);
	return minScore(1, opcolor,-10000, nowChessBoard);
}
int Reversi::maxScore(int len,int color,int theMinMax, char aChessBoard[][8])
{
	if (len == MAX_LENGTH)
	{
		return evalueChessBoard(ownColor, aChessBoard);
	}
	else if (getSNum(aChessBoard) == 0)
	{
		return evalueChessBoard(ownColor, aChessBoard);
	}
	else
	{
		int max;
		int SNum = 0;
		for (int i = 0; i<8; i++)
			for (int j = 0; j<8; j++)
				if (aChessBoard[i][j] == 'S')
				{
					char nowChessBoard[8][8];
					for (int i = 0; i < 8; i++)
						for (int j = 0; j < 8; j++)
							nowChessBoard[i][j] = aChessBoard[i][j];
					int opcolor = color ^ 1;
					fixAndReverseChess(i, j, color, nowChessBoard);
					releaseP(nowChessBoard);
					upDateP(i, j, nowChessBoard);
					releaseS(nowChessBoard);
					updateS(opcolor, nowChessBoard);
					if (SNum == 0)
						max = minScore(len + 1, opcolor, -10000, nowChessBoard);
					else
					{
						int score = minScore(len + 1, opcolor, max,nowChessBoard);
						if (max < score)
							max = score;
					}
					if (max > theMinMax)
						return max;
					SNum++;
				}
		return max;
	}
}
int Reversi::minScore(int len,int color,int theMaxMin, char aChessBoard[][8])
{
	if (len == MAX_LENGTH)
	{
		return evalueChessBoard(ownColor, aChessBoard);
	}
	else if (getSNum(aChessBoard) == 0)
	{
		return evalueChessBoard(ownColor, aChessBoard);
	}
	else
	{
		int min;
		int SNum=0;
		for(int i=0;i<8;i++)
			for(int j=0;j<8;j++)
				if (aChessBoard[i][j] == 'S')
				{
					char nowChessBoard[8][8];
					for (int i = 0; i < 8; i++)
						for (int j = 0; j < 8; j++)
							nowChessBoard[i][j] = aChessBoard[i][j];
					int opcolor = color ^ 1;
					fixAndReverseChess(i, j, color, nowChessBoard);
					releaseP(nowChessBoard);
					upDateP(i, j, nowChessBoard);
					releaseS(nowChessBoard);
					updateS(opcolor, nowChessBoard);
					if (SNum == 0)
						min = maxScore(len + 1, opcolor,10000, nowChessBoard);
					else
					{
						int score= maxScore(len + 1, opcolor,min, nowChessBoard);
						if (min > score)
							min = score;
					}
					if (min < theMaxMin)
						return min;
					SNum++;
				}
		return min;
	}
}
int Reversi::evalueChessBoard(int color,char aChessBoard[][8])
{
	//局面评估函数
	int blackScore = 0;
	int whiteScore = 0;
	for(int ro=0;ro<8;ro++)
		for (int co = 0; co < 8; co++)
		{
			if (aChessBoard[ro][co] == 'B')
			{
				if ((ro == 0 && co == 0) || (ro == 0 && co == 7) || (ro == 7 && co == 0) || (ro == 7 && co == 7))
				{
					if (nowStep < 26)
						blackScore += 64;
					else
						blackScore += 1;
				}
				else if (ro == 0 || ro == 7 || co == 0 || co == 7)
				{
					 if (nowStep < 26)
						blackScore += 8;
					else
						blackScore += 1;
				}
				else
					blackScore += 1;
			}
			else if (aChessBoard[ro][co] == 'W')
			{
				if ((ro == 0 && co == 0) || (ro == 0 && co == 7) || (ro == 7 && co == 0) || (ro == 7 && co == 7))
				{
					 if (nowStep < 26)
						whiteScore += 64;
					else
						whiteScore += 1;
				}
				else if (ro == 0 || ro == 7 || co == 0 || co == 7)
				{
					 if (nowStep < 26)
						whiteScore += 8;
					else
						whiteScore += 1;
				}
				else
					whiteScore += 1;
			}
		}
	if (color == 0)
		return (blackScore - whiteScore);
	else return (whiteScore - blackScore);
}
int Reversi::getSNum(char aChessBoard[][8])
{
	int SNum = 0;
	for (int ro = 0; ro<8; ro++)
		for (int co = 0; co<8; co++)
			if (chessBoard[ro][co] == 'S')
			{
				SNum++;
			}
	return SNum;
}
pair<int, int> Reversi::step()
{
	//此处写算法
	updateS(ownColor,chessBoard);
	int SNum = getSNum(chessBoard);
	if (SNum == 0)
		return make_pair(-1, -1);
	else 
	{ 
		int x = rtnBestPosition(SNum);
		releaseS(chessBoard);
		return make_pair(x/10, x%10); 
	}
}
void Reversi::updateS(int color,char aChessBoard[][8])
{
	for (int row = 0; row < 8; row++)
		for (int col = 0; col < 8; col++)
			if (ifS(row, col,color,aChessBoard))
				aChessBoard[row][col] = 'S';

}
void Reversi::releaseS(char aChessBoard[][8])
{
	for (int r = 0; r < 8; r++)
		for (int c = 0; c < 8; c++)
			if (aChessBoard[r][c] == 'S')
				aChessBoard[r][c] = 'O';
}
bool Reversi::ifS(int x, int y, int color, char aChessBoard[][8])
{
	if (aChessBoard[x][y] != 'O')
		return 0;
	//只有原本是O的地方才能落子
	if (ifUpleftReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifUpReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifUprightReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifLeftReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifRightReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifDownleftReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifDownReverseChess(x, y, color, aChessBoard))
		return 1;
	if (ifDownrightReverseChess(x, y, color, aChessBoard))
		return 1;
	return 0;
}
bool Reversi::ifUpleftReverseChess(int x, int y, int color, char aChessBoard[][8])
{
	if (x<2 || y<2)
		return 0;
	int m = x - 1;
	int n = y - 1;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		m--;
		n--;
		for (; m != -1 && n != -1 && aChessBoard[m][n] == 'W';)
		{
			m--;
			n--;
		}
		if (m == -1 || n == -1)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		m--;
		n--;
		for (; m != -1 && n != -1 && aChessBoard[m][n] == 'B';)
		{
			m--;
			n--;
		}
		if (m == -1 || n == -1)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifUpReverseChess(int x, int y, int color ,char aChessBoard[][8])
{
	if (x<2)
		return 0;
	int m = x - 1;
	int n = y;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		m--;
		for (; m != -1 && aChessBoard[m][n] == 'W';)
		{
			m--;
		}
		if (m == -1)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		m--;
		for (; m != -1 && aChessBoard[m][n] == 'B';)
		{
			m--;
		}
		if (m == -1)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifUprightReverseChess(int x, int y, int color, char aChessBoard[][8])
{
	if (x<2 || y>5)
		return 0;
	int m = x - 1;
	int n = y + 1;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		m--;
		n++;
		for (; m != -1 && n != 8 && aChessBoard[m][n] == 'W';)
		{
			m--;
			n++;
		}
		if (m == -1 || n == 8)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		m--;
		n++;
		for (; m != -1 && n != 8 && aChessBoard[m][n] == 'B';)
		{
			m--;
			n++;
		}
		if (m == -1 || n == 8)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifLeftReverseChess(int x, int y, int color, char aChessBoard[][8])
{
	if (y<2)
		return 0;
	int m = x;
	int n = y - 1;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		n--;
		for (; n != -1 && aChessBoard[m][n] == 'W';)
		{
			n--;
		}
		if (n == -1)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		n--;
		for (; n != -1 && aChessBoard[m][n] == 'B';)
		{
			n--;
		}
		if (n == -1)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifRightReverseChess(int x, int y, int color, char aChessBoard[][8])
{
	if (y>5)
		return 0;
	int m = x;
	int n = y + 1;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		n++;
		for (; n != 8 && aChessBoard[m][n] == 'W';)
		{
			n++;
		}
		if (n == 8)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		n++;
		for (; n != 8 && aChessBoard[m][n] == 'B';)
		{
			n++;
		}
		if (n == 8)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifDownleftReverseChess(int x, int y, int color, char aChessBoard[][8])
{
	if (x>5 || y<2)
		return 0;
	int m = x + 1;
	int n = y - 1;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		m++;
		n--;
		for (; m != 8 && n != -1 && aChessBoard[m][n] == 'W';)
		{
			m++;
			n--;
		}
		if (m == 8 || n == -1)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		m++;
		n--;
		for (; m != 8 && n != -1 && aChessBoard[m][n] == 'B';)
		{
			m++;
			n--;
		}
		if (m == 8 || n == -1)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifDownReverseChess(int x, int y, int color, char aChessBoard[][8])
{
	if (x>5)
		return 0;
	int m = x + 1;
	int n = y;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		m++;
		for (; m != 8 && aChessBoard[m][n] == 'W';)
		{
			m++;
		}
		if (m == 8)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		m++;
		for (; m != 8 && aChessBoard[m][n] == 'B';)
		{
			m++;
		}
		if (m == 8)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
bool Reversi::ifDownrightReverseChess(int x, int y, int color, char aChessBoard[][8])
{

	if (x>5 || y>5)
		return 0;
	int m = x + 1;
	int n = y + 1;
	if (color == 0)//black
	{
		if (aChessBoard[m][n] != 'W')
			return 0;
		m++;
		n++;
		for (; m != 8 && n != 8 && aChessBoard[m][n] == 'W';)
		{
			m++;
			n++;
		}
		if (m == 8 || n == 8)
			return 0;
		else if (aChessBoard[m][n] == 'B')
			return 1;
		else return 0;
	}
	if (color == 1)//white
	{
		if (aChessBoard[m][n] != 'B')
			return 0;
		m++;
		n++;
		for (; m != 8 && n != 8 && aChessBoard[m][n] == 'B';)
		{
			m++;
			n++;
		}
		if (m == 8 || n == 8)
			return 0;
		else if (aChessBoard[m][n] == 'W')
			return 1;
		else return 0;
	}
	else
	{
		cout << "something wrong when dealing the color !" << endl;
		return 0;
	}
}
void Reversi::saveChessBoard(int round, int step, int color)
{

	errno_t err;
	FILE *fp;
	char boardName[100] = "round ";
	//
	time_t curtime;
	char b[100];
	time(&curtime);
	ctime_s(b, 100, &curtime);
	int i = 0;
	while (b[i] != '\n')
	{
		if (b[i] == ':')
			b[i] = ' ';
		i++;
	}
	b[i] = '\0';
	//
	char address[100] = "\0";
	char a[4];
	a[0] = '0' + round;
	a[1] = '\0';
	strcat_s(boardName, a);
	strcat_s(boardName, " step ");
	if (step<10)
		a[0] = '0' + step;
	else
	{
		a[0] = step / 10 + '0';
		a[1] = step % 10 + '0';
		a[2] = '\0';
	}
	strcat_s(boardName, a);
	if (color == 0)
		strcat_s(boardName, " for black ");
	else
		strcat_s(boardName, " for White ");
	strcat_s(address, boardName);
	strcat_s(address, b);
	strcat_s(address, ".txt");
	err = fopen_s(&fp, address, "w");
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			fprintf(fp, "%c ", chessBoard[r][c]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	}
void Reversi::fixAndReverseChess(int row, int col, int color, char aChessBoard[][8])
{
	if (color == 0)//Black
	{
		aChessBoard[row][col] = 'B';
		if (ifUpleftReverseChess(row, col, 0,aChessBoard))//左上
		{
			int m = row - 1;
			int n = col - 1;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				m--;
				n--;
			}
		}
		if (ifUpReverseChess(row, col, 0, aChessBoard))//上
		{
			int m = row - 1;
			int n = col;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				m--;
			}
		}
		if (ifUprightReverseChess(row, col, 0, aChessBoard))//右上
		{
			int m = row - 1;
			int n = col + 1;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				m--;
				n++;
			}
		}
		if (ifLeftReverseChess(row, col, 0, aChessBoard))//左
		{
			int m = row;
			int n = col - 1;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				n--;
			}
		}
		if (ifRightReverseChess(row, col, 0, aChessBoard))//右
		{
			int m = row;
			int n = col + 1;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				n++;
			}
		}
		if (ifDownleftReverseChess(row, col, 0, aChessBoard))//左下
		{
			int m = row + 1;
			int n = col - 1;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				m++;
				n--;
			}
		}
		if (ifDownReverseChess(row, col, 0, aChessBoard))//下
		{
			int m = row + 1;
			int n = col;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				m++;
			}
		}
		if (ifDownrightReverseChess(row, col, 0, aChessBoard))//右下
		{
			int m = row + 1;
			int n = col + 1;
			while (aChessBoard[m][n] == 'W')
			{
				aChessBoard[m][n] = 'B';
				m++;
				n++;
			}
		}
	}
	else//白色
	{
		aChessBoard[row][col] = 'W';
		if (ifUpleftReverseChess(row, col, 1, aChessBoard))//左上
		{
			int m = row - 1;
			int n = col - 1;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				m--;
				n--;
			}
		}
		if (ifUpReverseChess(row, col, 1, aChessBoard))//上
		{
			int m = row - 1;
			int n = col;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				m--;
			}
		}
		if (ifUprightReverseChess(row, col, 1, aChessBoard))//右上
		{
			int m = row - 1;
			int n = col + 1;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				m--;
				n++;
			}
		}
		if (ifLeftReverseChess(row, col, 1, aChessBoard))//左
		{
			int m = row;
			int n = col - 1;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				n--;
			}
		}
		if (ifRightReverseChess(row, col, 1, aChessBoard))//右
		{
			int m = row;
			int n = col + 1;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				n++;
			}
		}
		if (ifDownleftReverseChess(row, col, 1, aChessBoard))//左下
		{
			int m = row + 1;
			int n = col - 1;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				m++;
				n--;
			}
		}
		if (ifDownReverseChess(row, col, 1, aChessBoard))//下
		{
			int m = row + 1;
			int n = col;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				m++;
			}
		}
		if (ifDownrightReverseChess(row, col, 1, aChessBoard))//右下
		{
			int m = row + 1;
			int n = col + 1;
			while (aChessBoard[m][n] == 'B')
			{
				aChessBoard[m][n] = 'W';
				m++;
				n++;
			}
		}
	}
}
void Reversi::handleMessage(int row, int col, int color)
{
	cout << "********************************************" << endl;
	cout << "in handleMessage: color: " << color << " " << "pos: " << "(" << row << "," << col << ")" << endl;
	if (row != -1 && col != -1)
		fixAndReverseChess(row, col, color,chessBoard);
		releaseP(chessBoard);
		upDateP(row, col,chessBoard);
}
void Reversi::releaseP(char aChessBoard[][8])
{
	for (int r = 0; r < 8; r++)
		for (int c = 0; c < 8; c++)
			if (aChessBoard[r][c] == 'P')
				aChessBoard[r][c] = 'O';
}
void Reversi::upDateP(int x, int y, char aChessBoard[][8])
{
	if (x != 0 && aChessBoard[x - 1][y] == 'O')
		aChessBoard[x - 1][y] = 'P';
	if (x != 7 && aChessBoard[x + 1][y] == 'O')
		aChessBoard[x + 1][y] = 'P';
	if (y != 7 && aChessBoard[x][y + 1] == 'O')
		aChessBoard[x][y + 1] = 'P';
	if (y != 0 && aChessBoard[x][y - 1] == 'O')
		aChessBoard[x][y - 1] = 'P';
}
void Reversi::showChessBoard(int Round, int step)
{
	cout << endl;
	cout << "This is the chessBoard in Round " << Round << " step " << step << endl;
	cout << "**********************************************************" << endl;
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
			cout << chessBoard[x][y] << " ";
		cout << endl;
	}
	cout << "***********************************************************" << endl;
}