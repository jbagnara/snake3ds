#include <string.h>
#include <3ds.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>


void block(int y, int x, int color, u8* fb, u8* fb1) //print snake
{
	//0 is B, 1 is G, 2 is R
        fb[3*(y+x*240)+color] = 0xFF;
	fb1[3*(y+x*240)+color] = 0xFF;
}

void remove(int y, int x, u8* fb, u8* fb1) //remove snake tail
{
	fb[3*(y+x*240)]=0x00;
	fb[3*(y+x*240)+1]=0x00;
	fb[3*(y+x*240)+2]=0x00;

	fb1[3*(y+x*240)]=0x00;
        fb1[3*(y+x*240)+1]=0x00;
        fb1[3*(y+x*240)+2]=0x00;
}

void wall(int limitstarty, int limitstartx, int limitendy, int limitendx, u8* fb, u8* fb1) //wallblock
{

    	for(int c = 0; c<240; c++)
        {
                for(int b = 0; b<400; b++)
                {
                	if((c<limitstarty||c>=limitendy)||(b<limitstartx||b>=limitendx)){
                		block(c, b, 0, fb, fb1);}
	        }
	}
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
	

int main()
{
	gfxInitDefault();
	//gfxSet3D(true);
	std::vector<int> xcords, ycords;
	int applex=0;
	int appley=0;
	int blocksize=10;
	int limitstarty=((240%(blocksize+1)-1)/2);
	int limitstartx=((400%(blocksize+1)-1)/2);
	int limitendy, limitendx;
	if((limitstarty*2)<(240%(blocksize+1)-1))
		limitendy=(239-limitstarty+1);
	else
		limitendy=(240-limitstarty);

	if((limitstartx*2)<(400%(blocksize+1)-1))
		limitendx=(399-limitstartx);
	else
		limitendx=(400-limitstartx);
	
	int y=limitstarty+1;
	int x=limitstartx+1;
	bool apple=1;
	bool up = 0;
	bool down = 0;
	bool left = 0;
	bool right = 0;
	bool gameover = 1;
	bool message=1;
	size_t score = 0;

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

	wall(limitstarty, limitstartx, limitendy, limitendx, fb, fb1);
	
	while (aptMainLoop())
	{
		
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown(); 
		if(gameover)
		{

			printf("\x1b[0;0Hscore: %d      ", score/2);
			printf("\x1b[0;15Hhighscore: %d", highscore);
			//( 240x400 and 240x320) 0x0 - 239x399, 239x319

			xcords.insert(xcords.begin(), x);
			ycords.insert(ycords.begin(), y);

			if(y<=limitstarty||y>=limitendy-blocksize||x<=limitstartx||x>=limitendx-blocksize)
			{	
				gameover=0;
				printf("\x1b[15;6H      Press B to quit       ");
			}

			if(score>3)
			{
				for(size_t c = 2;c<xcords.size(); c++)
				{
					if(x==xcords.at(c)&&y==ycords.at(c))
					{	
						gameover=0;
						printf("\x1b[15;6H      Press B to quit       ");
					}
				}
			}
		
			if(gameover)
			{
					
				//snake block
				for(int c = 0; c<blocksize; c++)
				{
					for(int d = 0; d<blocksize; d++)
					{
					
						block(ycords.at(0)+c, xcords.at(0)+d, 1, fb, fb1);
						if(!message)
							if(xcords.size()>(score+1))
								remove(ycords.back()+c, xcords.back()+d, fb, fb1);

					}
				}
				while(xcords.size()>(score+1))
				{
					xcords.pop_back();
					ycords.pop_back();
				}


				//apple block
				if(apple)
				{
					
					while(appley<=limitstarty||appley>=limitendy-blocksize||applex<=limitstartx||applex>=limitendx-blocksize)
					{
						appley=((rand()%(240/(blocksize+1)))*(blocksize+1)+(limitstarty+1));
						applex=((rand()%(400/(blocksize+1)))*(blocksize+1)+(limitstartx+1));
						
						for(size_t l = 0; l< xcords.size(); l++)
							if(appley==ycords.at(l)&&applex==xcords.at(l))
								{
									appley=0;
									applex=0;
								}
					}
					
					for(int c = 0; c<blocksize; c++)
					{
						for(int d = 0; d<blocksize; d++)
						{
							
							block(appley+c, applex+d, 2, fb, fb1);
						}
					}
					apple=0;
					
				}

				if(x==applex&&y==appley)
				{
					score+=2;
					apple=1;
					applex=0;
					appley=0;
					for(int c = 0; c<blocksize; c++)
					{
						for(int d = 0; d<blocksize; d++)
						{
							remove(y+c, x+d, fb, fb1);
							block(y+c, x+d, 1, fb, fb1);
						}
					}
				}

				if(kDown)
				{
					if(message)
						for(int c = 0; c<100; c++)
							printf("\n");
					
					message=0;
					if(((kDown & KEY_UP)||(kDown & KEY_X)) && !down)
					{	
						up=1;
						down=0;
						left=0;
						right=0;
					}

					else if(((kDown & KEY_DOWN)||(kDown & KEY_B)) && !up)
					{	
						down=1;
						up=0;
						left=0;
						right=0;
					}

					else if(((kDown & KEY_RIGHT)||(kDown & KEY_A)) && !left)
					{	
						right=1;
						up=0;
						left=0;
						down=0;
					}

					else if(((kDown & KEY_LEFT)||(kDown & KEY_Y)) && !right)
					{	
						left=1;
						up=0;
						down=0;
						right=0;
					}
				}

				if(left==1)
					x-=blocksize+1;
					

				else if(right==1)
					x+=blocksize+1;

				else if(up==1)
					y+=blocksize+1;

				else if(down==1)
					y-=blocksize+1;
			}
		}
		
		
		else
		{
			save(score);
			readscore.open ("snakesave.sav");
			readscore >> highscore;
			readscore.close();
			usleep(30000);	
			if(kDown)
			{
				if(kDown & KEY_B)
					break;

				gameover=1;
				x=limitstartx+1;
				y=limitstarty+1;
				score=0;
				xcords.clear();
				ycords.clear();
				
				memset(fb, 0, 240*400*3);
				memset(fb1, 0, 240*400*3);
				wall(limitstarty, limitstartx, limitendy, limitendx, fb, fb1);
				left=0;
				right=0;
				up=0;
				down=0;
				apple=1;
				applex=0;
				appley=0;
				message=1;
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
