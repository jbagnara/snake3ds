#include <string.h>
#include <3ds.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
int blockSize = 11;	//size of blocks by pixel
int xRes = 400;
int yRes = 240;

void drawPixel(int y, int x, int color, u8* fb, u8* fb1, u8 value) { //draw pixel of specified color
	//0 is B, 1 is G, 2 is R
        fb[3*(y+x*240)+color] = value;
	fb1[3*(y+x*240)+color] = value;
}

void drawBlock(int y, int x, int color, u8* fb, u8* fb1, u8 value) {	//fills block of pixels with color
	for(int b=0; b<blockSize; b++){
		for(int c=0; c<blockSize; c++){
			drawPixel(y+b, x+c, color, fb, fb1, value);
			
		}
	}
}

void eraseBlock(int y, int x, u8* fb, u8* fb1) {	//erases entire block
	drawBlock(y, x, 0, fb, fb1, 0x00);
	drawBlock(y, x, 1, fb, fb1, 0x00);
	drawBlock(y, x, 2, fb, fb1, 0x00);
}

void save(int score)
{
	int highscore;
	std::ifstream savefile;
	savefile.open("snakesave.sav");
	savefile >> highscore;
	savefile.close();
	if(highscore < (score/2))
	{
		std::ofstream writefile;
		remove("snakesave.sav");
		savefile.open("snakesave.sav");
		savefile.close();
		writefile.open("snakesave.sav");
		writefile << (score/2);
		writefile.close();
	}
}

class playField {
	public:
		playField(int, int);
		void init(u8*, u8*);

		int xLen;	//x resolution	(400 for 3ds)
		int yLen;	//y resulution	(240 for 3ds)
		int limitStartx, limitStarty;	//x, y coordinate where field starts
		int limitendy, limitendx;	//x, y coordinate where field ends
		int xArrLen, yArrLen;	//size of block array (field)

};

playField::playField(int xLen, int yLen){
	this->xLen = xLen;
	this->yLen = yLen;	
	limitStarty=((yLen%blockSize)/2);	
	limitStartx=((xLen%blockSize)/2);
	limitendy=limitStarty+(blockSize*(yLen/blockSize));
	limitendx=limitStartx+(blockSize*(xLen/blockSize));
	xArrLen=(limitStartx-limitendx)/blockSize;
	yArrLen=(limitStarty-limitendy)/blockSize;

	

}

void playField::init(u8* fb, u8* fb1){	//initializes playfield and draws walls
    	for(int y = 0; y<yLen; y++) {
                for(int x = 0; x<xLen; x++) {
                	if((y<limitStarty||y>=limitendy)||(x<limitStartx||x>=limitendx)){
                		drawPixel(y, x, 0, fb, fb1, 0xFF);
			}
	        }
	}
}

class gameState {
	public:
		int headx, heady;	//snake head coordinates
		std::vector<int> xcords, ycords;	//arrays of live snake block indexes
		int applex, appley;
		playField* field;
		gameState(int, int, playField*);
		void init();
		void newApple();

		bool apple=1;
		bool up = 0;
		bool down = 0;
		bool left = 0;
		bool right = 0;
		bool gameover = 1;
		bool message=1;
		size_t score = 0;
	
};	

gameState::gameState(int headx, int heady, playField* field){
	this->headx = headx;
	this->heady = heady;
	this->field = field;
	newApple();
}

void gameState::init(){
	apple=1;
	up=0;
	down=0;
	left=0;
	right=0;
	gameover=1;
	message=1;
	score = 0;
	
}

void gameState::newApple(){
	this->appley=((rand()%(field->yLen/(blockSize+1)))*(blockSize+1)+(field->limitStarty+1));
	this->applex=((rand()%(field->xLen/(blockSize+1)))*(blockSize+1)+(field->limitStartx+1));
}

int main()
{
	gfxInitDefault();
	//gfxSet3D(true);

	
	//int y=limitStarty+1;
	//int x=limitStartx+1;

	int highscore;
	std::ifstream readscore;
	readscore.open("snakesave.sav");
	readscore >> highscore;
	readscore.close();
	
	consoleInit(GFX_BOTTOM, NULL);

	printf("\x1b[15;6HPress any direction to play");
	u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
	gfxSwapBuffers();
	u8* fb1 = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);

	playField field (xRes, yRes);
	field.init(fb, fb1);
	gameState game (field.limitStartx+1, field.limitStarty+1, &field);
	
	while (1)
	{
		
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if(game.gameover)
		{

			printf("\x1b[0;0Hscore: %d      ", game.score/2);
			printf("\x1b[0;15Hhighscore: %d", highscore);
			//( 240x400 and 240x320) 0x0 - 239x399, 239x319

			game.xcords.insert(game.xcords.begin(), game.headx);
			game.ycords.insert(game.ycords.begin(), game.heady);

			if(game.heady<field.limitStarty||game.heady>field.limitendy-blockSize||game.headx<field.limitStartx||game.headx>field.limitendx-blockSize)	//head of snake has hit border
			{	
				game.gameover=0;
				printf("\x1b[15;6H      Press B to quit       ");
			}

			if(game.score>3)
			{
				for(size_t c = 2;c<game.xcords.size(); c++)
				{
					if(game.headx==game.xcords.at(c)&&game.heady==game.ycords.at(c))
					{	
						game.gameover=0;
						printf("\x1b[15;6H      Press B to quit       ");
					}
				}
			}
		
			if(game.gameover)
			{
					
				eraseBlock(game.ycords.back(), game.xcords.back(), fb, fb1);		//erases tail block

				eraseBlock(game.ycords.at(0), game.xcords.at(0), fb, fb1);		
				drawBlock(game.ycords.at(0), game.xcords.at(0), 1, fb, fb1, 0xFF);	//draws new head block

				while(game.xcords.size()>(game.score+1)){
					game.xcords.pop_back();
					game.ycords.pop_back();
				}


				//apple block
				if(game.apple)
				{
					
					while(game.appley<=field.limitStarty||game.appley>=field.limitendy-blockSize||game.applex<=field.limitStartx||game.applex>=field.limitendx-blockSize)
					{
						game.newApple();	
						for(size_t l = 0; l< game.xcords.size(); l++)
							if(game.appley==game.ycords.at(l)&&game.applex==game.xcords.at(l))
								{
									game.appley=0;
									game.applex=0;
								}
					}

					drawBlock(game.appley, game.applex, 2, fb, fb1, 0xFF);
					
					game.apple=0;
					
				}

				if(game.headx==game.applex&&game.heady==game.appley)	//apple get!
				{
					game.score+=2;
					game.apple=1;
					game.applex=0;
					game.appley=0;
					drawBlock(game.heady, game.headx, 1, fb, fb1, 0xFF);
				}

				if(kDown)
				{
					if(game.message)
						for(int c = 0; c<100; c++)
							printf("\n");
					
					game.message=0;
					if(((kDown & KEY_UP)||(kDown & KEY_X)) && !game.down)
					{	
						game.up=1;
						game.down=0;
						game.left=0;
						game.right=0;
					}

					else if(((kDown & KEY_DOWN)||(kDown & KEY_B)) && !game.up)
					{	
						game.down=1;
						game.up=0;
						game.left=0;
						game.right=0;
					}

					else if(((kDown & KEY_RIGHT)||(kDown & KEY_A)) && !game.left)
					{	
						game.right=1;
						game.up=0;
						game.left=0;
						game.down=0;
					}

					else if(((kDown & KEY_LEFT)||(kDown & KEY_Y)) && !game.right)
					{	
						game.left=1;
						game.up=0;
						game.down=0;
						game.right=0;
					}
				}

				if(game.left)
					game.headx-=blockSize+1;

				else if(game.right)
					game.headx+=blockSize+1;

				else if(game.up)
					game.heady+=blockSize+1;

				else if(game.down)
					game.heady-=blockSize+1;
			}
		}
		
		
		else
		{
			save(game.score);
			readscore.open ("snakesave.sav");
			readscore >> highscore;
			readscore.close();
			usleep(30000);	
			if(kDown)
			{
				if(kDown & KEY_B)
					break;

				game.gameover=1;
				game.headx=field.limitStartx+1;
				game.heady=field.limitStarty+1;
				game.score=0;
				game.xcords.clear();
				game.ycords.clear();
				
				memset(fb, 0, 240*400*3);
				memset(fb1, 0, 240*400*3);
				field.init(fb, fb1);
				game.init();
				usleep(300000);
				printf("\x1b[15;6HPress any direction to play");
			}
		}
		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
		usleep(50000);
	}
	gfxExit();
	return 0;
}
