//
//  Reversi.h
//  ReversiClient
//
//  Created by ganjun on 2018/3/6.
//  Copyright © 2018年 ganjun. All rights reserved.
//
#ifndef Reversi_h
#define Reversi_h
#include <stdio.h>
#include "ClientSocket.h"
using namespace std;
class Reversi 
{
private:
	int nowStep;
	ClientSocket client_socket;
	int ownColor;
	int oppositeColor;
	//function 
	void handleMessage(int row, int col, int color);
	// according to chessman position (row , col) , generate one step message in order to send to server
	void generateOneStepMessage(int row, int col);
	pair<int, int> step();
	char chessBoard[8][8];
	void updateS(int color,char aChessBoard[][8]);
	bool ifS(int x, int y,int color, char aChessBoard[][8]);
	bool ifUpleftReverseChess(int x,int y,int color, char aChessBoard[][8]);
	bool ifUpReverseChess(int x, int y,int color, char aChessBoard[][8]);
	bool ifUprightReverseChess(int x, int y,int color, char aChessBoard[][8]);
	bool ifLeftReverseChess(int x, int y,int color, char aChessBoard[][8]);
	bool ifRightReverseChess(int x, int y,int color, char aChessBoard[][8]);
	bool ifDownleftReverseChess(int x, int y,int color, char aChessBoard[][8]);
	bool ifDownReverseChess(int x, int y,int color, char aChessBoard[][8]);
	bool ifDownrightReverseChess(int x, int y,int color, char aChessBoard[][8]);
	int evaluePosition(int ro, int co,int color,char aChessBoard[][8]);
	int evalueChessBoard(int color, char aChessBoard[][8]);
	int maxScore(int len,int color,int theMinMax, char aChessBoard[][8]);//color是落子的颜色
	int minScore(int len,int color,int theMaxMin, char aChessBoard[][8]);//len是已递归了多少层
	int rtnBestPosition(int SNum);
	int getSNum(char aChessBoard[][8]);
	void releaseS(char aChessBoard[][8]);
	void fixAndReverseChess(int row, int col, int color, char aChessBoard[][8]);
	void releaseP(char aChessBoard[][8]);
	void upDateP(int x, int y, char aChessBoard[][8]);
	void showChessBoard(int Round,int step);
	void saveChessBoard(int round,int step,int color);
public:
	Reversi();
	~Reversi();
	void authorize(const char *id, const char *pass);
	void gameStart();
	void gameOver();
	void roundStart(int round);
	void oneRound(int Round);
	void roundOver(int round);
	int observe();
};

#endif /* Reversi_h */


