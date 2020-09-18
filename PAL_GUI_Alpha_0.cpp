#ifndef PAL_GUI_ALPHA_0_CPP
#define PAL_GUI_ALPHA_0_CPP 1

#include <vector>
#include <atomic>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "PAL_BasicFunctions_0.cpp"
/*
	Plan:
	Refactoring;
	
	New widgets:
		Progress Bar;
		Basic widget-TextSolve(Include display and edit);
		TableView;
		BlockView;
		TreeView;
		DownDragBox;
		Menu(Include Inner_Self_Draw_Version,Outside_Window_Draw_Version and System_API_Version);
		#TextEditLine(I haven't thought it should be so complex)
	
	Improvements:
		Replace some code with const,&;
		Using C++11 new features;
		#The ScrollBar doesn't sync with SimpleListView(or its LargeLayer) when using the mouse scroll;
		SimpleListView(or other similar widgets):
			Change storaging data in itself into storaging in outside;The SLV only provides the number of choose which event,then calls the function to solve it;
		Combine LargeLayer into Layer;
		RGBA & RGB;
		#!!! LargeLayerCombineWithScrollBar;(remove raw LargeLayer, Scroll Bar and something produced by them.)(I think it may improve a lot...)
		Remove Lslmt;
		Show,Check,CheckFocus,LoseFocus not virtual;
		SimpleTextBox;(Not so good);
			
	Solve Bugs:
		#Layer which neendn't show ScrollBar shows it after expanding and then shrinking the Window(or the Layer);
		

*/

//UI:
namespace PAL_GUI_Alpha//Version 1.0
{
	//using namespace std;
	
	PAL_Config_Alpha PUI_Cfg("PUI_Cfg.txt");
	
	string WindowTitle="";
	SDL_Window *PUI_Win;
	SDL_Renderer *PUI_Ren;
	TTF_Font *PUI_Font;
	SDL_Event *PUI_NowSolvingEvent=NULL;
	
	bool NeedFreshScreenFlag=1;
	bool NeedUpdatePosize=1; 
	bool EventSolvedFlag=0;
	bool DEBUG_DisplayBorderFlag=0;
	int PUI_WindowWidth=1280,PUI_WindowHeight=720;
	int WheelSensibility=40; 
	
	Uint32 PUI_EVENT_PRESENT=-1;
		   //PUI_VirtualPosEvent=-1;
	
	struct RGBA
	{
		Uint8 r,g,b,a=255;
		
		void SetColor(Uint8 _r,Uint8 _g,Uint8 _b,Uint8 _a)
		{r=_r;g=_g;b=_b;a=_a;}
		
		RGBA() {}
		
		RGBA(Uint8 _r,Uint8 _g,Uint8 _b,Uint8 _a)
		{r=_r;g=_g;b=_b;a=_a;}
		
		SDL_Color ToSDLColor()
		{
			SDL_Color re;
			re.r=r;re.g=g;re.b=b;re.a=a;
			return re;
		}
		
		bool HaveColor()
		{return a!=0;}
		
		bool operator == (RGBA _co)
		{return r==_co.a&&g==_co.g&&b==_co.b&a==_co.a;}
	};
	RGBA RGBA_WHITE={255,255,255,255},
		 RGBA_BLACK={0,0,0,255},
		 RGBA_NONE={0,0,0,0},
		 RGBA_BLUE_8A[8]={{0,100,255,31},{0,100,255,63},{0,100,255,95},{0,100,255,127},{0,100,255,159},{0,100,255,191},{0,100,255,223},{0,100,255,255}},
		 RGBA_GRAY_B8A[8]={{0,0,0,31},{0,0,0,63},{0,0,0,95},{0,0,0,127},{0,0,0,159},{0,0,0,191},{0,0,0,223},{0,0,0,255}},
		 RGBA_GRAY_W8A[8]={{255,255,255,31},{255,255,255,63},{255,255,255,95},{255,255,255,127},{255,255,255,159},{255,255,255,191},{255,255,255,223},{255,255,255,255}},
		 RGBA_BLUE_8[8]={{224,236,255,255},{192,217,255,255},{160,197,255,255},{128,178,255,255},{96,158,255,255},{64,139,255,255},{32,119,255,255},{0,100,255,255}},
		 RGBA_GRAY_8[8]={{224,224,224,255},{192,192,192,255},{160,160,160,255},{128,128,128,255},{96,96,96,255},{64,64,64,255},{32,32,32,255},{0,0,0,255}};
	
	template <typename T1,typename T2,typename T3> T1 EnsureInRange(T1 x,T2 L,T3 R) 
	{
		if (x<L) return L;
		else if (x>R) return R;
		else return x;
	}
	
	template <typename T1,typename T2,typename T3>  bool InRange(T1 x,T2 L,T3 R)
	{return L<=x&&x<=R;}
	
	struct Range
	{
		int l=0,r=0;
		
	};
	
	struct Point
	{
		int x,y;
		
		inline Point operator + (Point _pt)
		{return {x+_pt.x,y+_pt.y};}
		
		inline Point operator - (Point _pt)
		{return {x-_pt.x,y-_pt.y};}
		
	    inline bool operator == (Point _pt)
		{return x==_pt.x&&y==_pt.y;}
		
		inline int operator * (Point _pt)
		{return x*_pt.x+y*_pt.y;}
		
		inline int operator % (Point _pt)
		{return x*_pt.y-y*_pt.x;}
		
		void PrintPoint()
		{DD<<"PointPrint "<<x<<" "<<y<<"\n";}
		
		Point GetSDLEventPos(SDL_Event &event)
		{
			if (event.type==SDL_MOUSEBUTTONUP||event.type==SDL_MOUSEBUTTONDOWN)
				x=event.button.x,y=event.button.y;
			else if (event.type==SDL_MOUSEMOTION)
				x=event.motion.x,y=event.motion.y;
		}
	};
	
	struct Posize//很重要的结构体;不建议使用负的w,h 
	{
		int x,y,w,h;
		
		inline int x2()
		{return x+w-1;}
		
		inline int y2()
		{return y+h-1;}
		
		inline int midX()
		{return x+x2()>>1;}//??
		
		inline int midY()
		{return y+y2()>>1;}
		
		inline Point GetCentre()
		{return {midX(),midY()};}
		
		inline void SetCentre(Point pt)
		{
			x=pt.x-(w+1>>1)+1;
			y=pt.y-(h+1>>1)+1;
		}
		
		inline Point GetLU()
		{return {x,y};}
		
		inline Point GetLD()
		{return {x,y2()};}
		
		inline Point GetRU()
		{return {x2(),y};}
		
		inline Point GetRD()
		{return {x2(),y2()};}
		
		inline bool In(int xx,int yy)
		{return xx>=x&&xx<x+w&&yy>=y&&yy<y+h;}
		
		inline bool In(Point pt)
		{return pt.x>=x&&pt.x<x+w&&pt.y>=y&&pt.y<y+h;}
		
		inline void SetX_ChangeW(int _x)
		{w=x2()-_x+1;x=_x;}
		
		inline void SetY_ChangeH(int _y)
		{h=y2()-_y+1;y=_y;}
		
		inline void SetX2(int _x2)
		{w=_x2-x+1;}
		
		inline void SetY2(int _y2)
		{h=_y2-y+1;}
		
		inline void SetX2_ChangeX(int _x2)
		{x=_x2-w+1;}
		
		inline void SetY2_ChangeY(int _y2)
		{y=_y2-h+1;}
		
		inline void SetW_ChangeX(int _w)
		{x=x2()-_w+1;w=_w;}
		
		inline void SetH_ChangeY(int _h)
		{y=y2()-_h+1;h=_h;}
		
		inline Point EnsureIn(Point _pt)
		{
			_pt.x=max(_pt.x,x);
			_pt.x=min(_pt.x,x2());
			_pt.y=max(_pt.y,y);
			_pt.y=min(_pt.y,y2());
			return _pt;
		}
		
		inline Posize operator & (Posize _ps)
		{
			Posize re;
			re.x=max(x,_ps.x);
			re.y=max(y,_ps.y);
			re.w=max(0,min(x2(),_ps.x2())-re.x+1);
			re.h=max(0,min(y2(),_ps.y2())-re.y+1);
			return re;
		}
		
		inline Posize operator | (Posize _ps)
		{
			Posize re;
			re.x=min(x,_ps.x);
			re.y=min(y,_ps.y);
			re.w=min(0,max(x2(),_ps.x2())-re.x+1);
			re.h=min(0,max(y2(),_ps.y2())-re.y+1);
			return re;
		}
		
		inline Posize operator + (Posize _ps)
		{return {x+_ps.x,y+_ps.y,w,h};}
		
		inline Posize operator + (Point _pt)
		{return {x+_pt.x,y+_pt.y,w,h};}
		
		inline Posize operator - (Posize _ps)
		{return {x-_ps.x,y-_ps.y,w,h};}
		
		inline Posize operator - (Point _pt)
		{return {x-_pt.x,y-_pt.y,w,h};}
		
		inline bool operator == (Posize _ps)
		{return x==_ps.x&&y==_ps.y&&w==_ps.w&&h==_ps.h;}
	
		inline Posize Expand(int d)
		{return {x-d,y-d,w+2*d,h+2*d};}
		
		inline Posize Shrink(int d)
		{return {x+d,y+d,w-2*d,h-2*d};}
		
		inline Posize Flexible(double px,double py)
		{return {x*px,y*py,w*px,h*py};}
		
		inline SDL_Rect ToRect()
		{return {x,y,w,h};}
		
		void PrintPosize()
		{DD<<"PosizePrint "<<x<<" "<<y<<" "<<w<<" "<<h<<"\n";}
	};
	
	Point MakePoint(int x,int y)
	{return {x,y};}

	Posize MakePosize(int x,int y,int w,int h)
	{return {x,y,w,h};}
	
	Posize GetTexturePosize(SDL_Texture *tex)
	{
		Posize re;
		re.x=re.y=re.w=re.h=0;
		SDL_QueryTexture(tex,NULL,NULL,&re.w,&re.h);
		return re;
	}
	
	Point const ZERO_POINT={0,0};
	Point NowPos;
	Point WindowPos={100,100};
	
	Posize const ZERO_POSIZE={0,0,0,0};
	Posize PresentLimit;//It is not OK yet;
	
	void SetWindowPosition(const Point &pos)
	{
		WindowPos=pos;
		SDL_SetWindowPosition(PUI_Win,pos.x,pos.y);
		DD<<"SetWindowPosition "<<pos.x<<" "<<pos.y<<"\n";
	}
	
	void SetWindowSize(const Point &size)
	{
		PUI_WindowWidth=EnsureInRange(size.x,10,1e4);
		PUI_WindowHeight=EnsureInRange(size.y,10,1e4);
		SDL_SetWindowSize(PUI_Win,PUI_WindowWidth,PUI_WindowHeight);
		DD<<"SetWindowSize "<<size.x<<" "<<size.y<<"\n";
	}
	
	void SetWindowPosize(const Posize &ps)
	{
		WindowPos.x=ps.x;
		WindowPos.y=ps.y;
		PUI_WindowWidth=EnsureInRange(ps.w,10,1e4);
		PUI_WindowHeight=EnsureInRange(ps.h,10,1e4);
		SDL_SetWindowPosition(PUI_Win,ps.x,ps.y);
		SDL_SetWindowSize(PUI_Win,PUI_WindowWidth,PUI_WindowHeight);
		DD<<"SetWindowPosize "<<ps.x<<" "<<ps.y<<" "<<ps.w<<" "<<ps.h<<"\n";
	}
	
//	void SetVirtualPosEvent(Point pt)
//	{
//		SDL_Event event;
//	    SDL_UserEvent userevent;
//		
//	    userevent.type=PUI_VirtualPosEvent;
//	    userevent.code=0;
//	    userevent.data1=;
//	    userevent.data2=NULL;
//	    event.type=SDL_USEREVENT;
//	    event.user=userevent;
//	    
//	    SDL_PushEvent(&event);
//	}
	
	class Widgets;
	
	class PosizeEX
	{
		public:
			Widgets *wg=NULL;
			PosizeEX *nxt=NULL; 
			virtual void GetrPS()
			{
				
			}
			
			virtual ~PosizeEX()
			{
				delete nxt;
			}
	};
	
	RGBA GetSDLSurfacePixel(SDL_Surface *sur,Point pt)
	{
		if (!MakePosize(0,0,sur->w-1,sur->h-1).In(pt))
			return RGBA_NONE;
		Uint32 re;
		RGBA ret=RGBA_NONE;
//		//Uint32 getpixel(SDL_Surface*surface,int x,int y) //From net
//		//{
//			int bpp=;
//			Uint8 *p=(Uint8*)sur->pixels+pt.y*sur->pitch+pt.x*bpp;
//			switch (bpp)
//			{
//				case 1: re=*p;break;
//				case 2: re=*(Uint16*)p;break;
//				case 3:
//					if (SDL_BYTEORDER==SDL_BIG_ENDIAN)
//						re=p[0]<<16|p[1]<<8|p[2];
//					else
//						re=p[0]|p[1]<<8|p[2]<<16;
//					break;
//				case 4: re=*(Uint32*)p;
//				default: re=0;
//			}
//		//} 

		SDL_LockSurface(sur); 
		re=((Uint32*)sur->pixels)[(pt.y*sur->w+pt.x)];
		SDL_UnlockSurface(sur);
		SDL_GetRGBA(re,sur->format,&ret.r,&ret.g,&ret.b,&ret.a);
		return ret;
	}
	
	SDL_Texture *CreateRBGARectTexture(int w,int h,RGBA co)
	{
		SDL_Surface *sur=SDL_CreateRGBSurface(0,w,h,32,0,0,0,0);//efficiency??
		SDL_FillRect(sur,NULL,SDL_MapRGB(sur->format,co.r,co.g,co.b));
		SDL_SetSurfaceBlendMode(sur,SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(sur,co.a);
		SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
		SDL_FreeSurface(sur);
		return tex;
	}
	
	SDL_Surface *CreateRGBARectSurface(int w,int h,RGBA co)
	{
		SDL_Surface *sur=SDL_CreateRGBSurface(0,w,h,32,0,0,0,0);
		SDL_FillRect(sur,NULL,SDL_MapRGB(sur->format,co.r,co.g,co.b));
		SDL_SetSurfaceBlendMode(sur,SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(sur,co.a);
		return sur;
	}
	
	void RenderDrawLine(Point pt1,Point pt2)
	{SDL_RenderDrawLine(PUI_Ren,pt1.x,pt1.y,pt2.x,pt2.y);}
	
	void RenderDrawRectWithLimit(Posize ps,RGBA co,Posize lmt)//Posize &lmt ??
	{
		SDL_SetRenderDrawColor(PUI_Ren,co.r,co.g,co.b,co.a);
		lmt=lmt&ps;
//		if (lmt.In(ps.GetLU())||lmt.In(ps.GetRU()))//效率？？
//			RenderDrawLine(lmt.EnsureIn(ps.GetLU()),lmt.EnsureIn(ps.GetRU()));
//		if (lmt.In(ps.GetLU())||lmt.In(ps.GetLD()))
//			RenderDrawLine(lmt.EnsureIn(ps.GetLU()),lmt.EnsureIn(ps.GetLD()));
//		if (lmt.In(ps.GetLD())||lmt.In(ps.GetRD()))
//			RenderDrawLine(lmt.EnsureIn(ps.GetLD()),lmt.EnsureIn(ps.GetRD()));
//		if (lmt.In(ps.GetRU())||lmt.In(ps.GetRD()))
//			RenderDrawLine(lmt.EnsureIn(ps.GetRU()),lmt.EnsureIn(ps.GetRD()));
		
		if (lmt.h>0)
		{
			if (lmt.x==ps.x)
				RenderDrawLine(lmt.GetLU(),lmt.GetLD());
			if (lmt.x2()==ps.x2())
				RenderDrawLine(lmt.GetRU(),lmt.GetRD());
		}
		if (lmt.w>0)
		{
			if (lmt.y==ps.y)
				RenderDrawLine(lmt.GetLU(),lmt.GetRU());
			if (lmt.y2()==ps.y2())
				RenderDrawLine(lmt.GetLD(),lmt.GetRD());
		}
	}
	
	void RenderFillRect(Posize ps,RGBA co)
	{
		if (co.a==0) return;
		SDL_SetRenderDrawColor(PUI_Ren,co.r,co.g,co.b,co.a);
		SDL_Rect rct=ps.ToRect();
		SDL_RenderFillRect(PUI_Ren,&rct);
	}
	
	void RenderCopy(SDL_Texture *tex,Posize tex_PS,Posize targetPS)
	{
		if (tex_PS==ZERO_POSIZE) tex_PS=GetTexturePosize(tex);
		SDL_Rect srct=tex_PS.ToRect(),
			 	 drct=targetPS.ToRect();
		SDL_RenderCopy(PUI_Ren,tex,&srct,&drct);
	}
	
	void RenderCopy(SDL_Texture *tex,Posize tex_PS,Point pt)
	{
		if (tex_PS==ZERO_POSIZE) tex_PS=GetTexturePosize(tex);
		SDL_Rect srct=tex_PS.ToRect(),
			 	 drct={pt.x,pt.y,tex_PS.w,tex_PS.h};
		SDL_RenderCopy(PUI_Ren,tex,&srct,&drct);
	}
	
	void RenderCopyWithLmt(SDL_Texture *tex,Posize tex_gPS,Posize targetPS,Posize lmt)
	{
		//DD<<"RenderCopyWithLmt Cannot Use Yet!\n";
		Posize texPs=GetTexturePosize(tex);
		if (tex_gPS==ZERO_POSIZE) tex_gPS=texPs;
		SDL_Rect srct,drct;
		//srct=((lmt-targetPS).Flexible(texPs.w*1.0/targetPS.w,texPs.h*1.0/targetPS.h)&texPs).ToRect();
		srct=((lmt-targetPS).Flexible(texPs.w*1.0/targetPS.w,texPs.h*1.0/targetPS.h)&tex_gPS).ToRect();
		drct=(lmt&targetPS).ToRect();
		SDL_RenderCopy(PUI_Ren,tex,&srct,&drct);
	}
	
	void RenderCopyWithLmt(SDL_Texture *tex,Posize tex_gPS,Point pt,Posize lmt)
	{
		Posize texPs=GetTexturePosize(tex);
		if (tex_gPS==ZERO_POSIZE) tex_gPS=texPs;
		Posize targetPS={pt.x,pt.y,tex_gPS.w,tex_gPS.h};
		SDL_Rect srct,drct;
		//srct=((lmt-targetPS)&texPs).ToRect();
		srct=((lmt-targetPS)&tex_gPS).ToRect();
		drct=(lmt&targetPS).ToRect();
		SDL_RenderCopy(PUI_Ren,tex,&srct,&drct);
	}
	
	void RenderCopyWithLmtCentre(SDL_Texture *tex,Posize tex_PS,Posize targetPS,Posize lmt)
	{
//		Posize texPs=GetTexturePosize(tex);
//		if (tex_gPS==ZERO_POSIZE) tex_gPS=texPs;
//		SDL_Rect srct,drct;
//		srct=tex_PS.ToRect();
//		drct=targetPS.ToRect();
//		SDL_RenderCopy(PUI_Ren,tex,&srct,&drct);
		
		
//		Posize texPs=GetTexturePosize(tex);
//		if (tex_gPS==ZERO_POSIZE) tex_gPS=texPs;
//		SDL_Rect srct,drct;
//		srct=(lmt-targetPS)&(texPs+MakePoint()) 
//			((lmt-targetPS).Flexible(texPs.w*1.0/targetPS.w,texPs.h*1.0/targetPS.h)&texPs).ToRect();
//		drct=(lmt&targetPS).ToRect();
//		SDL_RenderCopy(PUI_Ren,tex,&srct,&drct);
//		
//		
//		
//		RenderCopy(tex,(lmt-gPS-MakePoint((rPS.w-sur->w)>>1,(rPS.h-sur->h)>>1))&MakePosize(0,0,sur->w,sur->h),
//								lmt&MakePosize(((rPS.w-sur->w)>>1)+gPS.x,((rPS.h-sur->h)>>1)+gPS.y,sur->w,sur->h));
	} 
	
	void Debug_DisplayBorder(Posize ps)
	{
		SDL_SetRenderDrawColor(PUI_Ren,255,0,0,200);
		SDL_RenderDrawLine(PUI_Ren,ps.x+5,ps.y,ps.x2()-5,ps.y);
		SDL_RenderDrawLine(PUI_Ren,ps.x+5,ps.y2(),ps.x2()-5,ps.y2());
		SDL_RenderDrawLine(PUI_Ren,ps.x,ps.y+5,ps.x,ps.y2()-5);
		SDL_RenderDrawLine(PUI_Ren,ps.x2(),ps.y+5,ps.x2(),ps.y2()-5);
		SDL_RenderDrawLine(PUI_Ren,ps.x+5,ps.y,ps.x,ps.y+5);
		SDL_RenderDrawLine(PUI_Ren,ps.x2()-5,ps.y,ps.x2(),ps.y+5);
		SDL_RenderDrawLine(PUI_Ren,ps.x+5,ps.y2(),ps.x,ps.y2()-5);
		SDL_RenderDrawLine(PUI_Ren,ps.x2()-5,ps.y2(),ps.x2(),ps.y2()-5);
		
		SDL_SetRenderDrawColor(PUI_Ren,0,0,255,200);
		SDL_RenderDrawLine(PUI_Ren,ps.x,ps.y,ps.x+4,ps.y);
		SDL_RenderDrawLine(PUI_Ren,ps.x,ps.y,ps.x,ps.y+4);
		SDL_RenderDrawLine(PUI_Ren,ps.x2(),ps.y,ps.x2()-4,ps.y);
		SDL_RenderDrawLine(PUI_Ren,ps.x2(),ps.y,ps.x2(),ps.y+4);
		SDL_RenderDrawLine(PUI_Ren,ps.x,ps.y2(),ps.x+4,ps.y2());
		SDL_RenderDrawLine(PUI_Ren,ps.x,ps.y2(),ps.x,ps.y2()-4);
		SDL_RenderDrawLine(PUI_Ren,ps.x2(),ps.y2(),ps.x2()-4,ps.y2());
		SDL_RenderDrawLine(PUI_Ren,ps.x2(),ps.y2(),ps.x2(),ps.y2()-4);
	}
	
	SDL_Surface *CreateRGBATextSurface(const char *text,RGBA co)
	{
		SDL_Surface *sur=TTF_RenderUTF8_Blended(PUI_Font,text,co.ToSDLColor());
		SDL_SetSurfaceBlendMode(sur,SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(sur,co.a);
		return sur;
	}
	
	SDL_Texture *CreateRGBATextTexture(const char *text,RGBA co)
	{
		SDL_Surface *sur=TTF_RenderUTF8_Blended(PUI_Font,text,co.ToSDLColor());
		SDL_SetSurfaceBlendMode(sur,SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(sur,co.a);
		SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
		SDL_FreeSurface(sur);
		return tex;
	}
	
	void RenderDrawTextWithLmt(string str,Posize targetPS,int mode,RGBA co,Posize lmt)//mode:-1:L 0:mid 1:R
	{
		lmt=lmt&targetPS;
		SDL_Surface *sur=CreateRGBATextSurface(str.c_str(),co);		
		SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
		if (mode==0)
			RenderCopy(tex,(lmt-targetPS-MakePoint((targetPS.w-sur->w)>>1,(targetPS.h-sur->h)>>1))&MakePosize(0,0,sur->w,sur->h),
						lmt&MakePosize(((targetPS.w-sur->w)>>1)+targetPS.x,((targetPS.h-sur->h)>>1)+targetPS.y,sur->w,sur->h));
		else if (mode==-1)
			RenderCopy(tex,(lmt-targetPS)&MakePosize(0,0,sur->w,sur->h),
						lmt&MakePosize(targetPS.x,((targetPS.h-sur->h)>>1)+targetPS.y,sur->w,sur->h));	
		else if (mode==1)
			RenderCopy(tex,(lmt-targetPS-MakePoint(targetPS.w-sur->w,(targetPS.h-sur->h)>>1))&MakePosize(0,0,sur->w,sur->h),
							lmt&MakePosize(targetPS.w-sur->w+targetPS.x,((targetPS.h-sur->h)>>1)+targetPS.y,sur->w,sur->h));
		SDL_FreeSurface(sur);
		SDL_DestroyTexture(tex);
	}
	
//	SDL_Texture *CreateRGBATextTexture_UNICODE(const wstring wstr,RGBA co)
//	{
//		SDL_Surface *sur=TTF_RenderUNICODE_Blended(PUI_Font,wstr.c_str(),co.ToSDLColor());
//		SDL_SetSurfaceBlendMode(sur,SDL_BLENDMODE_BLEND);
//		SDL_SetSurfaceAlphaMod(sur,co.a);
//		SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
//		SDL_FreeSurface(sur);
//		return tex;
//	}

	SDL_Surface *CreateTriangleSurface(int w,int h,Point pt1,Point pt2,Point pt3,RGBA co)
	{
		SDL_Surface *sur=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);
		SDL_SetSurfaceBlendMode(sur,SDL_BLENDMODE_BLEND);
		SDL_Rect rct;
		Uint32 col=SDL_MapRGBA(sur->format,co.r,co.g,co.b,co.a),col0=SDL_MapRGBA(sur->format,0,0,0,0);
		for (int i=0;i<h;++i)
		{
			int j,k;
			for (j=0;j<w;++j)
			{
				Point v0=MakePoint(j,i),
					  v1=pt1-v0,
					  v2=pt2-v0,
					  v3=pt3-v0;
				if (abs(v1%v2)+abs(v2%v3)+abs(v3%v1)==abs((pt2-pt1)%(pt3-pt1)))
					break;
			}
			for (k=w-1;k>=j;--k)
			{
				Point v0=MakePoint(k,i),
					  v1=pt1-v0,
					  v2=pt2-v0,
					  v3=pt3-v0;
				if (abs(v1%v2)+abs(v2%v3)+abs(v3%v1)==abs((pt2-pt1)%(pt3-pt1)))
					break;
			}
			if (j>0)
			{
				rct={0,i,j,1};
				SDL_FillRect(sur,&rct,col0);
			}
			if (k>j)
			{
				rct={j,i,k-j+1,1};
				SDL_FillRect(sur,&rct,col);
			}
			if (k<w-1)
			{
				rct={k+1,i,w-k-1,1};
				SDL_FillRect(sur,&rct,col0);
			}
		}
		return sur;
	}
	
	SDL_Texture *CreateTextureFromSurfaceAndDelete(SDL_Surface *sur)
	{
		SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
		SDL_FreeSurface(sur);
		return tex;
	}

	enum WidgetType
	{
		WgType_Widgets=0,
		
	};
	
	class Widgets
	{
		public:
			Posize rPS,gPS;//相对位置,全局位置(根据父控件的变化而更新)
			PosizeEX *PsEx=NULL;
			Point deltaPos={0,0};//子控件坐标偏移量
			bool visible;
			int ID=-1,type=0;
			Widgets *fa=NULL,*brother=NULL,
					*childWg=NULL;//,*SubLayer=NULL;
			
			virtual void Check(SDL_Event &event) {} 
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag) {} 
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
			}
			
			virtual void Show(Posize &lmt)
			{
				lmt=lmt&gPS;
			}
			
			virtual void ReceiveKeyboardInput(SDL_Event &event) {}//??
			
//			void CalcPS()
//			{
//				if (fa!=NULL)
//					gPS=rPS+fa->gPS+fa->deltaPos;
//				else gPS=rPS;
//			}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS();
//				CalcPS();//??
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
			}
			
//			void SetPsEx(PosizeEX *psex)
//			{
//				if (PsEx!=NULL) delete PsEx;
//				PsEx=psex;
//				PsEx->wg=this;
//				PsEx->GetrPS();
//				CalcPS();
//			}

			void AddPsEx(PosizeEX *psex)
			{
				if (PsEx!=NULL)
					psex->nxt=PsEx;
				PsEx=psex;
				PsEx->wg=this;
				CalcPsEx();
				NeedUpdatePosize=1;
			}
			
			void SetID(int _ID);
			
			void DeleteID();
			
			void ResetrPS(Posize ps)
			{
				if (PsEx==NULL)
				{
					rPS=ps;
					//CalcPS();
					CalcPsEx();
					//UpdateGlobalPos(childWg);
					NeedUpdatePosize=1;
				}else DD<<"Unable Reset rPS "<<ID<<". PosizeEX is not NULL.\n";
			}
			
			void Resize(int _w,int _h)
			{
				if (PsEx==NULL)
				{
					rPS.w=_w;rPS.h=_h;
					//CalcPS();
					CalcPsEx();
					//UpdateGlobalPos(childWg);
					NeedUpdatePosize=1;
				}else DD<<"Unable Reset rPS "<<ID<<". PosizeEX is not NULL.\n";
			}
			
			void Resize_R(int _w,int _h)
			{
				if (PsEx==NULL)
				{
					rPS.SetW_ChangeX(_w);
					rPS.SetH_ChangeY(_h);
					//CalcPS();
					CalcPsEx();
					//UpdateGlobalPos(childWg);
					NeedUpdatePosize=1;
				}else DD<<"Unable Reset rPS "<<ID<<". PosizeEX is not NULL.\n";
			}
			
			void ResizeDelta(int _w,int _h)
			{
				if (PsEx==NULL)
				{
					rPS.w+=_w;rPS.h+=_h;
					//CalcPS();
					CalcPsEx();
					//UpdateGlobalPos(childWg);
					NeedUpdatePosize=1;
				}else DD<<"Unable Resize "<<ID<<". PosizeEX is not NULL.\n";
			}
			
			void ResizeDelta_R(int _w,int _h)
			{
				if (PsEx==NULL)
				{
					rPS.w+=_w;rPS.h+=_h;
					rPS.x-=_w;rPS.y-=_h;
					//CalcPS();
					CalcPsEx();
					//UpdateGlobalPos(childWg);
					NeedUpdatePosize=1;
				}else DD<<"Unable Resize "<<ID<<". PosizeEX is not NULL.\n";
			}
			
			void ResetDeltaPos();
			
			
			virtual void _SolvePosEvent(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (MouseEventFlag!=-1)
					if (visible)
						if (gPS.In(pt))
						{
							if (childWg!=NULL) childWg->_SolvePosEvent(event,pt,MouseEventFlag);
							if (MouseEventFlag!=-1) CheckPos(event,pt,MouseEventFlag);
						}
				if (brother!=NULL) brother->_SolvePosEvent(event,pt,MouseEventFlag);
			}
			
			virtual void _SolveLoseFocusEvent(Point &pt,Posize lmt)
			{
				if (brother!=NULL) brother->_SolveLoseFocusEvent(pt,lmt);
				if (!visible) return;
				LoseFocus(pt,lmt);
				if (childWg!=NULL) childWg->_SolveLoseFocusEvent(pt,lmt);
			}
			
			virtual void _SolveEvent(SDL_Event &event)
			{
				if (brother!=NULL) brother->_SolveEvent(event);
				if (!visible) return;
				if (childWg!=NULL) childWg->_SolveEvent(event);
				if (EventSolvedFlag) return;
				Check(event);
			}
			
			virtual void _PresentWidgets(Posize lmt)
			{
				if (brother!=NULL) brother->_PresentWidgets(lmt);
				if (!visible) return;
				Show(lmt);
				//SDL_Delay(50);
				//SDL_RenderPresent(PUI_Ren);
				if (childWg!=NULL) childWg->_PresentWidgets(lmt);
			}
			
			virtual ~Widgets()
			{
				DD<<"Delete Widgets "<<ID<<"\n";
				delete PsEx;
				while (childWg!=NULL)
					delete childWg;
				if (fa->childWg==this)
					fa->childWg=brother;
				else 
					for (Widgets *i=fa->childWg;i!=NULL;i=i->brother)
						if (i->brother==this)
						{
							i->brother=brother;
							break;
						}
				DeleteID();
				NeedFreshScreenFlag=1;
				//DD<<"It should not be here:~Widgets() ID:"<<ID<<"\n";
			}
	};
	
	Widgets *OccupyPosWd=NULL;
	int OccupyPosState=0;
//	vector <pair <Widgets*,int> > OccupyPosWd;

	Widgets *KeyboardInputWg=NULL;
	int KeyboardInputWgState=0;
	
	map <int,Widgets*> WgID;
	
	void Widgets::SetID(int _ID)
	{
		map <int,Widgets*>::iterator p=WgID.find(_ID);
		if (p!=WgID.end()) DD<<"ID Occupied :"<<_ID<<" "<<(p->first)<<"\n";
		else WgID[_ID]=this,ID=_ID;
	}
	
	void Widgets::DeleteID()
	{
		WgID.erase(ID);
	}
		
//	virtual Widgets::~Widgets()
//	{
//		WgID.erase(ID);
//		delete PsEx;
//		//brother??
//		//child??
//	}

	void Widgets::ResetDeltaPos()
	{
		if (!(deltaPos==ZERO_POINT))
		{
			deltaPos=ZERO_POINT;
			//UpdateGlobalPos(SubLayer);
			//UpdateGlobalPos(childWg);
			NeedUpdatePosize=1;
		}
	}

	struct PUI_UpdateTimerData
	{
		Widgets *tar=NULL;
		int cnt=0;//>=0:use same interval cnt times(at this time stack is empty) -1:use same interval infinite times  -2:use intervals in stack;
		stack <Uint32> sta;
		atomic_bool *enableFlag=NULL;
	};
			
	Uint32 PUI_UpdateTimer(Uint32 interval,void *param)
	{
	    PUI_UpdateTimerData *p=(PUI_UpdateTimerData*)param;
		DD<<"PUI_UpdateTimer: WgID "<<(p->tar->ID)<<" cnt "<<(p->cnt)<<"\n";
	    
	    if (p->enableFlag!=NULL)
	    	if (*(p->enableFlag)==0)
	    	{
	    		delete p;
	    		return 0;
			}
			
	    SDL_Event event;
	    SDL_UserEvent userevent;
		
	    userevent.type=PUI_EVENT_PRESENT;
	    userevent.code=p->cnt==-2?p->sta.size():p->cnt;
	    userevent.data1=p->tar;
	    userevent.data2=NULL;
	    event.type=SDL_USEREVENT;
	    event.user=userevent;
	    
	    SDL_PushEvent(&event);
	    
	    if (p->cnt==-2)
		    if (p->sta.empty())
			{
				delete p;
				*(p->enableFlag)=0;
		    	return 0;
			}
			else
			{
				Uint32 t=p->sta.top();
				p->sta.pop();
				return t;
			}
	    else if (p->cnt==-1)
	    	return interval;
	    else 
			if (p->cnt==0)
		    {
		    	delete p;
		    	*(p->enableFlag)=0;
		    	return 0;
		    }
		    else 
			{
				--p->cnt;
				return interval;	
			}
	}
	
	void BuildRelation(Widgets *_fa,Widgets *now)
	{
		now->fa=_fa;
		now->brother=_fa->childWg;
		_fa->childWg=now;
	}
	
	class PosizeEX_Fa6:public PosizeEX
	{
		public:
			Uint8 xNotConsider=0,yNotConsider=0,faDep=1;
			double ra,rb,rc,rd;
			
			virtual void GetrPS()
			{
				if (nxt!=NULL) nxt->GetrPS();
				Posize &ps=wg->rPS,&psfa=wg->fa->rPS;
					
				double a=ra<0?(-ra)*psfa.w:ra,
					   b=rb<0?(-rb)*psfa.w:rb,
					   c=rc<0?(-rc)*psfa.h:rc,
					   d=rd<0?(-rd)*psfa.h:rd;//??
				switch (xNotConsider)
				{
					case 1: ps.w=a;ps.SetX2_ChangeX(psfa.w-b);break;
					case 2: ps.x=a;ps.SetX2(psfa.w-b);break;
					case 3: ps.x=a;ps.w=b;break;
				}
				
				switch (yNotConsider)
				{
					case 1: ps.h=c;ps.SetY2_ChangeY(psfa.h-d);break;
					case 2: ps.y=c;ps.SetY2(psfa.h-d);break;
					case 3: ps.y=c;ps.h=d;break;
				}
			}
			
			PosizeEX_Fa6(Uint8 _xn,Uint8 _yn,double a,double b,double c,double d)
			{
				xNotConsider=_xn;yNotConsider=_yn;faDep=1;
				if (!InRange(xNotConsider,1,3)) DD<<"PosizeEX Error: xNotConsiderValue set "<<xNotConsider<<"\n";
				if (!InRange(yNotConsider,1,3)) DD<<"PosizeEX Error: yNotConsiderValue set "<<yNotConsider<<"\n";
				ra=a;rb=b;rc=c;rd=d;
			}
	};
	
	class PosizeEX_BindrPS:public PosizeEX//If not W/H. Please ensure target and this have same father
	{
		public:
			enum WhichValue
			{
				NONE=0,
				X_CW=1,		X_CX2=2,	Y_CH=4,		Y_CY2=8,
				W_CX=16,	W_CX2=32,	H_CY=64,	H_CY2=128,
				X2_CX=256,	X2_CW=512,	Y2_CY=1024,	Y2_CH=2048
			};
			int Which=NONE;
			Widgets *target=NULL;
			
			virtual void GetrPS()
			{
				if (nxt!=NULL) nxt->GetrPS();
				if (Which&X_CW) wg->rPS.SetX_ChangeW(target->rPS.x);
				else if (Which&X_CX2) wg->rPS.x=target->rPS.x;
				if (Which&Y_CH) wg->rPS.SetY_ChangeH(target->rPS.y);
				else if (Which&Y_CY2) wg->rPS.y=target->rPS.y;
				if (Which&W_CX) wg->rPS.SetW_ChangeX(target->rPS.w);
				else if (Which&W_CX2) wg->rPS.w=target->rPS.w;
				if (Which&H_CY) wg->rPS.SetH_ChangeY(target->rPS.h);
				else if (Which&H_CY2) wg->rPS.h=target->rPS.h;
				if (Which&X2_CW) wg->rPS.SetX2(target->rPS.x2());
				else if (Which&X2_CX) wg->rPS.SetX2_ChangeX(target->rPS.x2());
				if (Which&Y2_CH) wg->rPS.SetY2(target->rPS.y2());
				else if (Which&Y2_CY) wg->rPS.SetY2_ChangeY(target->rPS.y2());
			}
			
			PosizeEX_BindrPS(Widgets *_tar,int _which)
			{target=_tar;Which=_which;}
	};
	
	class PosizeEX_BindrPS_2:public PosizeEX
	{
		public:
			enum WhichValue
			{	
				NONE=0,
				X,	Y,	W,	H,	X2,	Y2,
				X_CW,  X_CX2, Y_CH,  Y_CY2,
				W_CX,  W_CX2, H_CY,  H_CY2,
				X2_CX, X2_CW, Y2_CY, Y2_CH	
			
			};
			WhichValue WhichThat=NONE,WhichThis=NONE;
			Widgets *target=NULL;
			int ShiftValue=0;
			
			virtual void GetrPS()
			{
				if (nxt!=NULL) nxt->GetrPS();
				int val;
				switch (WhichThat)
				{
					case X:val=target->rPS.x;break;
					case Y:val=target->rPS.y;break;
					case W:val=target->rPS.w;break;
					case H:val=target->rPS.h;break;
					case X2:val=target->rPS.x2();break;
					case Y2:val=target->rPS.y2();break;
					default:
						DD<<"PosizeEX_BindrPSWith_2: Error WhichThat Value!\n";
						return;
				}
				val+=ShiftValue;
				switch (WhichThis)
				{
					case X_CW: 	wg->rPS.SetX_ChangeW(val);	break;
					case X_CX2:	wg->rPS.x=val;				break;
					case Y_CH:	wg->rPS.SetY_ChangeH(val);	break;
					case Y_CY2:	wg->rPS.y=val;				break;
					case W_CX:	wg->rPS.SetW_ChangeX(val);	break;
					case W_CX2:	wg->rPS.w=val;				break;
					case H_CY:	wg->rPS.SetH_ChangeY(val);	break;
					case H_CY2:	wg->rPS.h=val;				break;
					case X2_CX:	wg->rPS.SetX2_ChangeX(val);	break;
					case X2_CW:	wg->rPS.SetX2(val);			break;
					case Y2_CY:	wg->rPS.SetY2_ChangeY(val);	break;
					case Y2_CH:	wg->rPS.SetY2(val);			break;
					default:DD<<"PosizeEX_BindrPSWith_2: Error WhichThis Value!\n";
				}
			} 
			
			PosizeEX_BindrPS_2(Widgets *_tar,WhichValue _whichthis,WhichValue _whichthat,int _shiftvalue)
			{target=_tar;WhichThis=_whichthis;WhichThat=_whichthat;ShiftValue=_shiftvalue;}
			
			PosizeEX_BindrPS_2(Widgets *_tar,WhichValue _whichthis,WhichValue _whichthat)
			{target=_tar;WhichThis=_whichthis;WhichThat=_whichthat;}
	};
	
	class PosizeEX_MidFa:public PosizeEX
	{
		public:
			Point CenterShift;
			
			virtual void GetrPS()
			{
				if (nxt!=NULL) nxt->GetrPS();
				Posize &ps=wg->rPS,&psfa=wg->fa->rPS;
				
				ps.x=(psfa.w-ps.w>>1)+CenterShift.x;
				ps.y=(psfa.h-ps.h>>1)+CenterShift.y;
			}
			
			PosizeEX_MidFa(Point centreshift)
			{CenterShift=centreshift;}
	};
	
	class PosizeEX_LimitSizeRange:public PosizeEX
	{
		public:
			int min_w,max_w,min_h,max_h;
			
			virtual void GetrPS()
			{
				if (nxt!=NULL) nxt->GetrPS();
				Posize &ps=wg->rPS;
				
				if (min_w!=-1&&ps.w<min_w) ps.w=min_w;
				else if (max_w!=-1&&ps.w>max_w) ps.w=max_w;
				if (min_h!=-1&&ps.h<min_h) ps.h=min_h;
				else if (max_h!=-1&&ps.w>max_h) ps.h=max_h;
			}
			
			PosizeEX_LimitSizeRange(int _min_w,int _max_w,int _min_h,int _max_h)
			{min_w=_min_w;max_w=_max_w;min_h=_min_h;max_h=_max_h;}
	};
	
	class Layer:public Widgets
	{
		public:
			RGBA LayerColor=RGBA_NONE;
			
			void SetLayerColor(RGBA co)
			{LayerColor=co;}
			
			virtual void Show(Posize &lmt)
			{
				RenderFillRect(lmt=gPS&lmt,LayerColor);
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			Layer(int _ID,bool _visible,Widgets *_fa,Posize _rPS)//指相对fa的posize 
			{
				DD<<"Create Layer "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				if (_fa!=NULL)
				BuildRelation(_fa,this);
//					fa=_fa,brother=_fa->SubLayer,
//					_fa->SubLayer=this;
				rPS=_rPS;//CalcPS();
				CalcPsEx();
			}
	};
	
	Layer *MenusLayer=NULL;
	
//	class WindowLayer:
//	{
//		
//	}
	
//	class ChangePosPercent
//	{
//		public:
//			int WW,HH,ww,hh;
//			double PerW,PerH;
//			
//			virtual void ChangePP(int ope,double val);
//	};
	
//	class LargeLayer:public Widgets,public ChangePosPercent
//	{
//		public:
//			RGBA LayerColor=RGBA_NONE;
//			
//			void SetLayerColor(RGBA co)
//			{LayerColor=co;}
//			
//			virtual void ChangePP(int ope,double val)//ope:: 0:None 1:SetPosX 2:..Y 3:SetPosPercent 4:..Y 5:MoveX 6:..Y
//			{
//				switch (ope)
//				{
//					case 1: deltaPos.x=-val; break;
//					case 2: deltaPos.y=-val; break;
//					case 3: deltaPos.x=(rPS.w-WW)*val; break;
//					case 4: deltaPos.y=(rPS.h-HH)*val; break;
//					case 5:	deltaPos.x+=val; break;
//					case 6: deltaPos.y+=val; break;
//					default:
//						return;
//				}
//				deltaPos=MakePosize(rPS.w-WW,rPS.h-HH,WW-rPS.w+1,HH-rPS.h+1).EnsureIn(deltaPos);
//				PerW=deltaPos.x*1.0/(ww-WW-1);
//				PerH=deltaPos.y*1.0/(hh-HH-1);
//				//UpdateGlobalPos(SubLayer);
//				//UpdateGlobalPos(childWg);
//				NeedUpdatePosize=1;
//				NeedFreshScreenFlag=1;
//			}
//			
//			void ResizeLL(int _ww,int _hh)
//			{
//				if (_ww!=-1) WW=EnsureInRange(_ww,ww,1e9);
//				if (_hh!=-1) HH=EnsureInRange(_hh,hh,1e9);
//				NeedUpdatePosize=1;
//			}
//			
//			virtual void CalcPsEx()
//			{
//				if (PsEx!=NULL) PsEx->GetrPS(),WW=max(WW,rPS.w),HH=max(HH,rPS.h),ww=rPS.w,hh=rPS.h;
////				CalcPS();//??
//				if (fa!=NULL)
//					gPS=rPS+fa->gPS+fa->deltaPos;
//				else gPS=rPS;
//			}
//			
//			virtual void Show(Posize &lmt)
//			{
//				RenderFillRect(lmt=gPS&lmt,LayerColor);
//				if (DEBUG_DisplayBorderFlag)
//					Debug_DisplayBorder(gPS),
//					Debug_DisplayBorder({gPS.x+deltaPos.x,gPS.y+deltaPos.y,WW,HH});
//			}
//			
//			LargeLayer(int _ID,bool _visible,Widgets *_fa,Posize _rPS,int _WW,int _HH)
//			{
//				DD<<"Create LargeLayer\n";
//				SetID(_ID);visible=_visible;
//				if (_fa!=NULL)
//				BuildRelation(_fa,this);
////					fa=_fa,brother=_fa->SubLayer,
////					_fa->SubLayer=this;
//				rPS=_rPS;WW=_WW;HH=_HH;ww=rPS.w;hh=rPS.h;//CalcPS();
//				CalcPsEx();
//			}
//	};
	
	class EventLayer:public Widgets
	{
		public:
			void (*func)(void*,int,SDL_Event*,Point*,int*,Posize*)=NULL;//int1: 1.Check 2.CheckPos 3.LoseFocus
			void *funcData=NULL;
			
			void SetFunc(void (*_func)(void*,int,SDL_Event*,Point*,int*,Posize*))
			{func=_func;}
			
			void SetFuncData(void *_funcdata)
			{funcData=_funcdata;}
			
			virtual void Check(SDL_Event &event)
			{
				if (func!=NULL)
					func(funcData,1,&event,NULL,NULL,NULL);
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (func!=NULL)
					func(funcData,2,&event,&pt,&MouseEventFlag,NULL);
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				if (func!=NULL)
					func(funcData,3,NULL,&pt,NULL,&lmt);
			}
			
			virtual void Show(Posize &lmt)
			{
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			EventLayer(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create Layer "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				if (_fa!=NULL)
				BuildRelation(_fa,this);
				rPS=_rPS;
				CalcPsEx();
			}
	};
	
	class LargeLayerWithScrollBar:public Widgets
	{
		public:
			Posize ckPs_w,ckPs_h,ckBGps_h,ckBGps_w;
			Point ckCentreDelta={0,0};
			int WW,HH;
			int ScrollBarWidth=16;
			int stat=0;//0:Up_NoFocus 1:Up_Focus_Bar_H 2:Up_Focus_Bar_W 3:Up_Focus_Chunk_H 4:Up_Focus_Chunk_W
					   //5:Down_Scroll_Chunk_H(OccupyPos) 6:Down_Scroll_Chunk_W(O..P..) 7:Up_Focus_LargeLayer__NotFocus_Bar
			bool Not_W=0,Not_H=0;
			RGBA LayerColor=RGBA_NONE,
				 BorderColor[5]={{0,100,255,63},{0,100,255,127},{0,100,255,191},{0,100,255,255},{0,100,255,127}},//stat+1>>1
				 BackGroundBarColor={0,0,0,20},
				 ChunkColor[5]={{0,100,255,63},{0,100,255,127},{0,100,255,191},{0,100,255,255},{0,100,255,127}};
			
			void SetLayerColor(RGBA co)
			{LayerColor=co;}
			
			void SetBorderColor(int p,RGBA co)
			{BorderColor[p]=co;}
			
			void SetChunkColor(int p,RGBA co)
			{ChunkColor[p]=co;}
			
			void SetBackGroundColor(RGBA co)
			{BackGroundBarColor=co;}
			
			void UpdateChunkPs()
			{
				if (rPS.h>=HH) ckBGps_h=ckPs_h=ZERO_POSIZE;//??
				else 
				{
					//ckPs_h.y-ckBGps_h.y-2  /  ckBGps_h-4-ckPs_h.h == -deltaPos.y/HH-rPS.h;
					//ckPs_h.y-ckBGps_h.y-2  /  ckBGps_h-4-ckPs_h.h == -deltaPos.y/HH-rPS.h+ScrollBarWidth*!Not_W;
					ckBGps_h={gPS.x2()-ScrollBarWidth+1,gPS.y,ScrollBarWidth,gPS.h};
					ckPs_h=ckBGps_h.Shrink(2);
					ckPs_h.h=EnsureInRange(ckBGps_h.h*gPS.h*1.0/HH,5,ckBGps_h.h-4);//??
					//ckPs_h.y=(ckBGps_h.h-4-ckPs_h.h)*(-deltaPos.y)*1.0/(HH-gPS.h)+ckBGps_h.y+2;
					ckPs_h.y=(ckBGps_h.h-4-ckPs_h.h)*(-deltaPos.y)*1.0/(HH-gPS.h+ScrollBarWidth*!Not_W)+ckBGps_h.y+2;
				}
				if (rPS.w>=WW) ckBGps_w=ckPs_w=ZERO_POSIZE;
				else
				{
					ckBGps_w={gPS.x,gPS.y2()-ScrollBarWidth+1,rPS.h<HH?gPS.w-ScrollBarWidth:gPS.w,ScrollBarWidth};
					ckPs_w=ckBGps_w.Shrink(2);
					ckPs_w.w=EnsureInRange(ckBGps_w.w*gPS.w*1.0/WW,5,ckBGps_w.w-4);
					//ckPs_w.x=(ckBGps_w.w-4-ckPs_w.w)*(-deltaPos.x)*1.0/(WW-gPS.w)+ckBGps_w.x+2;
					ckPs_w.x=(ckBGps_w.w-4-ckPs_w.w)*(-deltaPos.x)*1.0/(WW-gPS.w+ScrollBarWidth*!Not_H)+ckBGps_w.x+2;
				}
			}
			
			void SetViewPort(int ope,double val)//ope:: 0:None 1:SetPosX 2:..Y 3:SetPosPercent 4:..Y 5:MoveX 6:..Y
			{
				switch (ope)
				{
					case 1: deltaPos.x=-val; break;
					case 2: deltaPos.y=-val; break;
					case 3: deltaPos.x=(rPS.w-WW)*val; break;
					case 4: deltaPos.y=(rPS.h-HH)*val; break;
					case 5:	deltaPos.x-=val; break;
					case 6: deltaPos.y-=val; break;
					default:
						return;
				}
				deltaPos=MakePosize(rPS.w-WW-ScrollBarWidth*!Not_H,rPS.h-HH-ScrollBarWidth*!Not_W,WW-rPS.w+1+ScrollBarWidth*!Not_H,HH-rPS.h+1+ScrollBarWidth*!Not_W).EnsureIn(deltaPos);
				//deltaPos=MakePosize(rPS.w-WW,rPS.h-HH,WW-rPS.w+1,HH-rPS.h+1).EnsureIn(deltaPos);
				UpdateChunkPs();
				NeedUpdatePosize=1;
				NeedFreshScreenFlag=1;
			}
			
			void ResizeLL(int _ww,int _hh)
			{
				//Not_W=_ww==0;Not_H=_hh==0;
				if (_ww!=-1) WW=EnsureInRange(_ww,rPS.w/*-deltaPos.x*/,1e9);//?? +1 -1??
				if (_hh!=-1) HH=EnsureInRange(_hh,rPS.h/*-deltaPos.y*/,1e9);
				Not_W=WW==rPS.w;Not_H=HH==rPS.h;
				deltaPos.x=max(rPS.w-WW,deltaPos.x);
				deltaPos.y=max(rPS.h-HH,deltaPos.y);
				UpdateChunkPs();
				NeedUpdatePosize=1;
			}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS(),WW=Not_W?rPS.w:max(WW,rPS.w),HH=Not_H?rPS.h:max(HH,rPS.h);//??deltaPos??
//				CalcPS();//??
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				UpdateChunkPs();
			}
			
			virtual void Check(SDL_Event &event)
			{
				if (event.type==SDL_MOUSEWHEEL)
				{
					switch (stat)
					{
						case 1:	case 3:	case 7:
							DD<<"Wheel H \n";
							if (HH>rPS.h)
								SetViewPort(6,-event.wheel.y*WheelSensibility);
							break;
						case 2:	case 4:
							DD<<"Wheel W \n";
							if (WW>rPS.w)
								SetViewPort(5,-event.wheel.y*WheelSensibility);
							break;
					}
				}
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					if (event.button.button==SDL_BUTTON_LEFT)
						if (ckBGps_h.In(pt))
						{
							DD<<"Sc.Down H"<<ID<<"\n";
							stat=5;MouseEventFlag=-1;
							OccupyPosState=1;
							OccupyPosWd=this;
							if (ckPs_h.In(pt)) ckCentreDelta.y=pt.y-ckPs_h.midY();
							else ckCentreDelta.y=0,SetViewPort(4,(pt.y-ckPs_h.h/2-ckBGps_h.y-2)*1.0/(ckBGps_h.h-4-ckPs_h.h));		
							NeedFreshScreenFlag=1;				
						}
						else if (ckBGps_w.In(pt))
						{
							DD<<"Sc.Down W"<<ID<<"\n";
							stat=6;MouseEventFlag=-1;
							OccupyPosState=1;
							OccupyPosWd=this;
							if (ckPs_w.In(pt)) ckCentreDelta.x=pt.x-ckPs_w.midX();
							else ckCentreDelta.x=0,SetViewPort(3,(pt.x-ckPs_w.w/2-ckBGps_w.x-2)*1.0/(ckBGps_w.w-4-ckPs_w.w));
							NeedFreshScreenFlag=1;
						}
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&(stat==5||stat==6))
				{
					DD<<"Sc.Up "<<ID<<"\n";
					if (ckPs_h.In(pt)) stat=3;
					else if (ckPs_w.In(pt)) stat=4;
					else if (ckBGps_w.In(pt)) stat=2;
					else if (ckBGps_h.In(pt)) stat=1;
					else stat=7;
					OccupyPosState=0;
					OccupyPosWd=NULL;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION)
				{
					bool MouseEventFlagChangeFlag=1;
					int temp_stat=-1;
					if (stat==5) SetViewPort(4,(pt.y-ckCentreDelta.y-ckPs_h.h/2-ckBGps_h.y-2)*1.0/(ckBGps_h.h-4-ckPs_h.h));
					else if (stat==6) SetViewPort(3,(pt.x-ckCentreDelta.x-ckPs_w.w/2-ckBGps_w.x-2)*1.0/(ckBGps_w.w-4-ckPs_w.w));
					else if (ckPs_h.In(pt)) temp_stat=3;
					else if (ckPs_w.In(pt)) temp_stat=4;
					else if (ckBGps_w.In(pt)) temp_stat=2;
					else if (ckBGps_h.In(pt)) temp_stat=1;
					else temp_stat=7,MouseEventFlagChangeFlag=0;
					
					if (MouseEventFlagChangeFlag) MouseEventFlag=-1;
					if (temp_stat!=-1&&temp_stat!=stat)
						stat=temp_stat,NeedFreshScreenFlag=1,DD<<"Sc.Focus "<<ID<<"\n";
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat!=0)
					if (!lmt.In(pt))
						stat=0,NeedFreshScreenFlag=1;
			}
			
			virtual void Show(Posize &lmt)
			{
				//DD<<"deltapos "<<deltaPos.x<<" "<<deltaPos.y<<"\n";
				if (HH>rPS.h)
				{
					//UpdateChunkPosize();??
					int col=stat&1?stat+1>>1:0;
					RenderFillRect(lmt&ckBGps_h,RGBA_WHITE);
					RenderFillRect(lmt&ckBGps_h,BackGroundBarColor);
					RenderDrawRectWithLimit(ckBGps_h,BorderColor[col],lmt);
					RenderFillRect(lmt&ckPs_h,ChunkColor[col]);
				}
				if (WW>rPS.w)
				{
					int col=stat&1?0:(stat+1>>1);
					RenderFillRect(lmt&ckBGps_w,RGBA_WHITE);
					RenderFillRect(lmt&ckBGps_w,BackGroundBarColor);
					RenderDrawRectWithLimit(ckBGps_w,BorderColor[col],lmt);
					RenderFillRect(lmt&ckPs_w,ChunkColor[col]);
				}
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS),
					Debug_DisplayBorder({gPS.x+deltaPos.x,gPS.y+deltaPos.y,WW,HH}),
					Debug_DisplayBorder(ckBGps_h),
					Debug_DisplayBorder(ckBGps_w);
			}
			
			virtual void _SolvePosEvent(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (MouseEventFlag!=-1)
					if (visible)
						if (gPS.In(pt))
						{
							if (MouseEventFlag!=-1) CheckPos(event,pt,MouseEventFlag);
							if (childWg!=NULL) childWg->_SolvePosEvent(event,pt,MouseEventFlag);
						}
				if (brother!=NULL) brother->_SolvePosEvent(event,pt,MouseEventFlag);
			}
			
			virtual void _PresentWidgets(Posize lmt)
			{
				if (brother!=NULL) brother->_PresentWidgets(lmt);
				if (!visible) return;
				RenderFillRect(lmt=gPS&lmt,LayerColor);
				if (childWg!=NULL) childWg->_PresentWidgets(lmt);
				Show(lmt);
			}
			
			~LargeLayerWithScrollBar()
			{
				DD<<"Delete LargeLayerWithScrollBar "<<ID<<"\n";
				if (stat==5||stat==6)
				{
					OccupyPosState=0;
					OccupyPosWd=NULL;
				}
			}
			
			LargeLayerWithScrollBar(int _ID,bool _visible,Widgets *_fa,Posize _rPS,int _WW,int _HH)
			{
				DD<<"Create LargeLayerWithScrollBar "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				if (_fa!=NULL)
					BuildRelation(_fa,this);
				rPS=_rPS;
				if (_WW<=rPS.w) WW=rPS.w,Not_W=1;
				else WW=_WW;
				if (_HH<=rPS.h) HH=rPS.h,Not_H=1;
				HH=_HH;//CalcPS();
				CalcPsEx();
				UpdateChunkPs();
			}
	};
	
	class Button:public Widgets
	{
		public:
			string text;
			int down=0;//0:Up_NoFocus 1:Up_Focus 2:Down
			void (*func)(void*)=NULL;
			void *funcData=NULL;
			RGBA ButtonColor[3]=//{{255,0,0,127},{255,0,0,160},{0,0,255,127},
								{{0,100,255,127},{0,100,255,160},{0,100,255,255}}, 
				 TextColor=RGBA_BLACK;
			
			void SetTextColor(RGBA co)
			{TextColor=co;}
			
			void SetButtonColor(int p,RGBA co)
			{ButtonColor[p]=co;}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"Button Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt))
							down=2,MouseEventFlag=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&down==2)
				{
					DD<<"Button Up "<<ID<<"\n";
					if (gPS.In(pt))
					{
						if (func!=NULL)
							func(funcData);
						down=1;
					}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&down==0)
				{
					DD<<"Button Focus "<<ID<<"\n";
					if (gPS.In(pt))
						down=1;
					NeedFreshScreenFlag=1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (down!=0)
					if (!lmt.In(pt))
						down=0,NeedFreshScreenFlag=1,DD<<"Button LoseFocus "<<ID<<"\n";
			}
			
			virtual void Show(Posize &lmt)
			{
				RenderFillRect(lmt=gPS&lmt,ButtonColor[down]);
				SDL_Surface *sur=CreateRGBATextSurface(text.c_str(),{0,0,0,255});
				SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
				RenderCopy(tex,(lmt-gPS)&MakePosize(0,0,sur->w,sur->h),
							lmt&MakePosize(((rPS.w-sur->w)>>1)+gPS.x,((rPS.h-sur->h)>>1)+gPS.y,sur->w,sur->h));
				SDL_FreeSurface(sur);
				SDL_DestroyTexture(tex);

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
					
			Button(int _ID,bool _visible,Widgets *_fa,Posize _rPS,string str,void(*_func)(void*),void *_funcData)
			{
				DD<<"Create Button "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				text=str;func=_func;
				if (_funcData==CONST_THIS) funcData=this;
				else funcData=_funcData;
			}
	};
	
	class ShapedPictureButton:public Widgets
	{
		public:
			string text;
			bool OnlyOnePic=1,//only use pic[0],pic1 and 2 auto create;(not usable yet)
				 ThroughBlankPixel=1;
			int down=0;//0:Up_NoFocus 1:Up_Focus 2:Down
			int ThroughLmtValue=0;//rgba.a not greater than this will through
			void (*func)(void*)=NULL;
			void *funcData=NULL;
			SDL_Texture *pic[3]={NULL,NULL,NULL};
			SDL_Surface *sur0=NULL;
//			RGBA ButtonColor[3]=//{{255,0,0,127},{255,0,0,160},{0,0,255,127},
//								{{0,100,255,127},{0,100,255,160},{0,100,255,255}}, 
			RGBA TextColor=RGBA_BLACK;
			
			void SetOpt(bool onlyonepic,bool throughblankpixel)
			{
				OnlyOnePic=onlyonepic;
				ThroughBlankPixel=throughblankpixel;
			}
			
			void SetTextColor(RGBA co)
			{TextColor=co;}
			
//			void SetButtonPicture(int p,SDL_Texture *tex)
//			{
//				
//			}

			void SetButtonPicture(int p,string str)
			{
				SDL_Surface *sur=IMG_Load(str.c_str());
				SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
				if (p!=0) SDL_FreeSurface(sur);
				else
				{
					if (sur0!=NULL) SDL_FreeSurface(sur0);
					sur0=sur;
				}
				if (pic[p]!=NULL) SDL_DestroyTexture(pic[p]);
				pic[p]=tex;
			}
			
//			void SetButtonColor(int p,RGBA co)
//			{ButtonColor[p]=co;}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"PictureButton Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt)&&(!ThroughBlankPixel||GetSDLSurfacePixel(sur0,pt-gPS.GetLU()).a>ThroughLmtValue))
							down=2,MouseEventFlag=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&down==2)
				{
					DD<<"PictureButton Up "<<ID<<"\n";
					if (gPS.In(pt)&&(!ThroughBlankPixel||GetSDLSurfacePixel(sur0,pt-gPS.GetLU()).a>ThroughLmtValue))
					{
						if (func!=NULL)
							func(funcData);
						down=1;
					}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&down==0)
				{
					DD<<"PictureButton Focus "<<ID<<"\n";
					if (gPS.In(pt)&&(!ThroughBlankPixel||GetSDLSurfacePixel(sur0,pt-gPS.GetLU()).a>ThroughLmtValue))
						down=1;
					NeedFreshScreenFlag=1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (down!=0)
					if (!(lmt.In(pt)&&(!ThroughBlankPixel||GetSDLSurfacePixel(sur0,pt-gPS.GetLU()).a>ThroughLmtValue)))
						down=0,NeedFreshScreenFlag=1,DD<<"PictureButton LoseFocus "<<ID<<"\n";
			}
			
			virtual void Show(Posize &lmt)
			{
				lmt=gPS&lmt;
			//	RenderFillRect(,ButtonColor[down]);
				RenderCopyWithLmt(pic[down],ZERO_POSIZE,gPS,lmt);
				SDL_Surface *sur=CreateRGBATextSurface(text.c_str(),{0,0,0,255});
				SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
				RenderCopy(tex,(lmt-gPS)&MakePosize(0,0,sur->w,sur->h),
							lmt&MakePosize(((rPS.w-sur->w)>>1)+gPS.x,((rPS.h-sur->h)>>1)+gPS.y,sur->w,sur->h));
				SDL_FreeSurface(sur);
				SDL_DestroyTexture(tex);

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			virtual ~ShapedPictureButton()
			{
				DD<<"Delete PictureButton "<<ID<<"\n";
				for (int i=0;i<=2;++i)
					SDL_DestroyTexture(pic[i]);
				SDL_FreeSurface(sur0);
			}
					
			ShapedPictureButton(int _ID,bool _visible,Widgets *_fa,Posize _rPS,string str,void(*_func)(void*),void *_funcData)
			{
				DD<<"Create PictureButton "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				text=str;func=_func;
				if (_funcData==CONST_THIS) funcData=this;
				else funcData=_funcData;
			}
	};
	
	class TinyText:public Widgets
	{
		public:
			int Mode=0;//0:mid -1:Left 1:Right
			string text;
			bool AutoW=0;
			RGBA TextColor=RGBA_BLACK,
				 BackgroundColor=RGBA_NONE;
			
			void SetTextColor(RGBA co)
			{TextColor=co;}
			
			void SetBackgroundColor(RGBA co)
			{BackgroundColor=co;}
			
			void SetText(string str)
			{
				text=str;
				if (AutoW)
					TTF_SizeUTF8(PUI_Font,str.c_str(),&rPS.w,NULL),CalcPsEx();
			}
			
			bool SetAutoW(bool on)
			{
				AutoW=on;
				if (AutoW)
					TTF_SizeUTF8(PUI_Font,text.c_str(),&rPS.w,NULL),CalcPsEx();
			}
			
			virtual void Show(Posize &lmt)
			{
				lmt=lmt&gPS;
				RenderFillRect(lmt,BackgroundColor);
				SDL_Surface *sur=CreateRGBATextSurface(text.c_str(),TextColor);
				SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
				if (Mode==0)
					RenderCopy(tex,(lmt-gPS-MakePoint((rPS.w-sur->w)>>1,(rPS.h-sur->h)>>1))&MakePosize(0,0,sur->w,sur->h),
								lmt&MakePosize(((rPS.w-sur->w)>>1)+gPS.x,((rPS.h-sur->h)>>1)+gPS.y,sur->w,sur->h));
				else if (Mode==-1)
					RenderCopy(tex,(lmt-gPS)&MakePosize(0,0,sur->w,sur->h),
								lmt&MakePosize(gPS.x,((rPS.h-sur->h)>>1)+gPS.y,sur->w,sur->h));	
				else if (Mode==1)
					RenderCopy(tex,(lmt-gPS-MakePoint(rPS.w-sur->w,(rPS.h-sur->h)>>1))&MakePosize(0,0,sur->w,sur->h),
									lmt&MakePosize(rPS.w-sur->w+gPS.x,((rPS.h-sur->h)>>1)+gPS.y,sur->w,sur->h));
				SDL_FreeSurface(sur);
				SDL_DestroyTexture(tex);
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			TinyText(int _ID,bool _visible,Widgets *_fa,Posize _rPS,string str,int _mode)
			{
				DD<<"Create TinyText "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				text=str;Mode=_mode;
			}
	};
	
	class CheckBox:public Widgets
	{
		public:
			bool on=0;
			int stat=0;//0:Up_NoFocus 1:Up_Focus 2:Down
			void (*func)(void*,bool)=NULL;
			void *funcData=NULL;
			RGBA BorderColor[3]=//{{255,0,0,127},{255,0,0,200},{0,0,255,127}},
								{{0,100,255,127},{0,100,255,160},{0,100,255,255}},
				 ChooseColor=//{0,0,255,127};
				 			 {0,100,255,127};
			
			void SetBorderColor(int p,RGBA co)
			{BorderColor[p]=co;}
			
			void SetChooseColor(RGBA co)
			{ChooseColor=co;}
			
			virtual void Show(Posize &lmt)
			{
				lmt=lmt&gPS;
				RenderDrawRectWithLimit(gPS,BorderColor[stat],lmt);
				if (on) RenderFillRect(lmt&gPS.Shrink(3),ChooseColor);
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt))
							stat=2,MouseEventFlag=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat==2)
				{
					if (gPS.In(pt))
					{
						on=!on,stat=1;
						if (func!=NULL)
							func(funcData,on);
					}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==0)
				{
					if (gPS.In(pt))
						stat=1;
					NeedFreshScreenFlag=1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat!=0)
					if (!lmt.In(pt))
						stat=0,NeedFreshScreenFlag=1;
			}
			
			CheckBox(int _ID,bool _visible,Widgets *_fa,Posize _rPS,bool defaultOnOff,void(*_func)(void*,bool),void *_funcData)
			{
				DD<<"Create CheckBox "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				on=defaultOnOff;
				func=_func;
				if (_funcData==CONST_THIS) funcData=this;
				else funcData=_funcData;
			}
	};
	
	class SimpleListView:public Widgets
	{
		public:
			//bool Align=0; 
			int stat=0;//0:Up_NoFocus 1:Up_Focus_Row 2:Down_Left 3:Down_Right 4:LeftClick_Choose_NotFunc
			int ListCnt=0;
			vector <string> ListText;
			vector <void*> FuncData;
			void (*func)(void*,int,int)=NULL;//int1:Pos   int2: 0:None 1:Left_Click 2:Left_Double_Click 3:Right_Click
			void (*DeleteFunc)(void*)=NULL;
			LargeLayerWithScrollBar *fa2=NULL;//改起来巨简单 
			int ChooseWhich1=-1,ChooseWhich2=-1;//1:FocusChoose 2:Click_Choose
			int EachHeight=20,
				Interval=2;
			RGBA TextColor=RGBA_BLACK,
				 RowColor[3]={{200,200,200,80},{0,100,255,100},{0,100,255,200}};//NoFocusRow,FocusRow,ClickRow
			
			void SetTextColor(RGBA co)
			{TextColor=co;}
			
			void SetRowColor(int p,RGBA co)
			{RowColor[p]=co;}
			
			void SetBackGroundColor(RGBA co)
			{fa2->LayerColor=co;}
			
			void SetEachHeight(int _x)
			{EachHeight=_x;}
			
			void SetInterval(int _x)
			{Interval=_x;}
			
			void SetListFunc(void (*_func)(void*,int,int))
			{func=_func;}
			
			void SetDeletFunc(void (*_deletefunc)(void*))
			{DeleteFunc=_deletefunc;}
			
			void SetListContent(int p,string str,void *_funcdata)//p: 0<=p<ListCnt:SetInP >=ListCnt:SetInLast <0:SetInFirst
			{
				p=EnsureInRange(p,0,ListCnt);
				ListText.insert(ListText.begin()+p,str);
				FuncData.insert(FuncData.begin()+p,_funcdata);
				++ListCnt;
				rPS.h=EnsureInRange(ListCnt*(EachHeight+Interval)-Interval,fa2->rPS.h,1e9);
				fa2->ResizeLL(-1,rPS.h);
				//CalcPS();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			void DeleteListContent(int p)//p: 0<=p<ListCnt:SetInP >=ListCnt:SetInLast <0:SetInFirst
			{
				if (!ListCnt) return;
				p=EnsureInRange(p,0,ListCnt-1);
				if (ChooseWhich2==p) ChooseWhich2=-1;//??
				if (DeleteFunc!=NULL) DeleteFunc(FuncData[p]);//??
				ListText.erase(ListText.begin()+p);
				FuncData.erase(FuncData.begin()+p);
				--ListCnt;
				rPS.h=EnsureInRange(ListCnt*(EachHeight+Interval)-Interval,fa2->rPS.h,1e9);
				fa2->ResizeLL(-1,rPS.h);
				//CalcPS();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			void ClearListContent()
			{
				if (!ListCnt) return;
				DD<<"Clear List "<<ListCnt<<"\n";
				ChooseWhich2=-1;//??
				if (DeleteFunc!=NULL)
					for (vector <void*>::iterator p=FuncData.begin();p!=FuncData.end();++p)
						DeleteFunc(*p),DD<<".";//??
						DD<<"\n";
				ListText.clear();
				FuncData.clear();
				ListCnt=0;
				rPS.h=fa2->rPS.h;
				fa2->ResizeLL(-1,rPS.h);
				//fa2->ResetDeltaPos();//??
				fa2->SetViewPort(1,0);
				fa2->SetViewPort(2,0);
				//CalcPS();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS();
				rPS.h=EnsureInRange(ListCnt*(EachHeight+Interval)-Interval,fa2->rPS.h,1e9);
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				fa2->ResizeLL(rPS.w,rPS.h);
			}
			
//			virtual void Check(SDL_Event &event)
//			{
//				if (event.type==SDL_KEYDOWN)
//					if (event.key.keysym.sym==SDLK_INSERT)
//						SetListContent(1e9,"Choice "+llTOstr(ListCnt),new string("Str "+llTOstr(ListCnt)));
//					else if (event.key.keysym.sym==SDLK_DELETE)
//						DeleteListContent(1e9);
//			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (ListCnt==0) return;
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					ChooseWhich1=(pt.y-gPS.y)/(EachHeight+Interval);
					if (InRange(ChooseWhich1,0,ListCnt-1)&&
						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight))
					{	
						if (event.button.button==SDL_BUTTON_LEFT)
							stat=4,MouseEventFlag=-1;
						else if (event.button.button==SDL_BUTTON_RIGHT)
							stat=3,MouseEventFlag=-1;
							
						if (event.button.clicks==2)
						{
							if (event.button.button==SDL_BUTTON_LEFT)
								if (ChooseWhich1==ChooseWhich2)
									stat=2,ChooseWhich2=-1;
						}
						else if (event.button.clicks==1,1)//??
							ChooseWhich2=ChooseWhich1;
							
						//DD<<"Click "<<(int)event.button.clicks<<"\n";
						NeedFreshScreenFlag=1;
					}
					else stat=0,ChooseWhich1=ChooseWhich2=-1,NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat!=0&&stat!=1)
				{	
					ChooseWhich1=(pt.y-gPS.y)/(EachHeight+Interval);
					if (InRange(ChooseWhich1,0,ListCnt-1)&&
						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight))
					{
						if (func!=NULL)
							if (stat==2) func(FuncData[ChooseWhich1],ChooseWhich1/*???*/,2);
							else if (stat==3) func(FuncData[ChooseWhich1],ChooseWhich1,3);
							else if (stat==4) func(FuncData[ChooseWhich2],ChooseWhich1,1);
						stat=1;
					}
					else stat=0,ChooseWhich1=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==0)
				{
					ChooseWhich1=(pt.y-gPS.y)/(EachHeight+Interval);
					if (InRange(ChooseWhich1,0,ListCnt-1)&&
						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight))
						stat=1,NeedFreshScreenFlag=1;
					else stat=0,ChooseWhich1=-1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat!=0)
				{
					if (!lmt.In(pt)||!(InRange(ChooseWhich1,0,ListCnt-1)&&
						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight)))
						stat=0,ChooseWhich1=-1,NeedFreshScreenFlag=1;
				}
			}
			
			virtual void Show(Posize &lmt)
			{
				if (ListCnt==0) return;
				int ForL=(-fa2->deltaPos.y)/(EachHeight+Interval),
					ForR=ForL+(fa2->gPS.h)/(EachHeight+Interval)+1;
				ForL=EnsureInRange(ForL,0,ListCnt-1);
				ForR=EnsureInRange(ForR,0,ListCnt-1);
				if (!InRange(ChooseWhich1,0,ListCnt-1)) ChooseWhich1=-1;
				if (!InRange(ChooseWhich2,0,ListCnt-1)) ChooseWhich2=-1;
//				DD<<"SimpleListView "<<ID<<"\n"
//				  <<"Display "<<"ForL "<<ForL<<" ForR "<<ForR<<"\n"
//				  <<"	ChooseWhich1 "<<ChooseWhich1<<"\n"
//				  <<"	ChooseWhich2 "<<ChooseWhich2<<"\n"
//				  <<"	Stat "<<stat<<"\n";
				Posize RowPs=MakePosize(gPS.x,gPS.y+ForL*(EachHeight+Interval),gPS.w,EachHeight);
				for (int i=ForL;i<=ForR;RowPs.y+=EachHeight+Interval,++i)
				{
					RenderFillRect(RowPs&lmt,RowColor[ChooseWhich2==i?2:(ChooseWhich1==i?(stat==3||stat==4?2:stat):0)]);
					SDL_Surface *sur=CreateRGBATextSurface(ListText[i].c_str(),TextColor);
					SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
					Posize _pps;
					RenderCopy(tex,(lmt-RowPs)&MakePosize(0,0,sur->w,sur->h),
									_pps=lmt&MakePosize(RowPs.x+5,((RowPs.h-sur->h)>>1)+RowPs.y,sur->w,sur->h));
					if (DEBUG_DisplayBorderFlag)
						Debug_DisplayBorder(_pps);
					SDL_FreeSurface(sur);
					SDL_DestroyTexture(tex);
				}	
					
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			void AddPsEx(PosizeEX *psex)
			{
				if (fa2->PsEx!=NULL)
					psex->nxt=fa2->PsEx;
				fa2->PsEx=psex;
				fa2->PsEx->wg=fa2;
				fa2->CalcPsEx();
				NeedUpdatePosize=1;
			}
			
			~SimpleListView()
			{
				DD<<"Delete SimpleListView "<<ID<<"\n";
				ClearListContent();
			}
			
			SimpleListView(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create SimpleListView "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				fa2=new LargeLayerWithScrollBar(ID*100000+1,1,_fa,_rPS,_rPS.w,_rPS.h);
				fa2->SetLayerColor({0,0,0,20});
				BuildRelation(fa2,this);
				rPS={0,0,_rPS.w,0};//CalcPS();
				Widgets::AddPsEx(new PosizeEX_BindrPS(fa2,PosizeEX_BindrPS::W_CX2));
			}
	};
	
	class PictureWg:public Widgets
	{
		public:
			SDL_Texture *src=NULL;
			void (*func)(void*,int)=NULL;//int:1:Left_Click 2:Right_Click 
			void *funcData=NULL; 
			Posize srcPS={0,0};
			RGBA BackGroundColor=RGBA_BLACK;
			int stat=0;//0:Up 1:Down_Left 2:Down_Right
			//bool Full;
			int Mode=0;//0:FullFill rPS 1:Initial 1:1_Pixel,change rPS,resizeable 2: 1:1_Pixel(Not change rPS) 
			double per=1;
			
			void SetBackGroundColor(RGBA co)
			{BackGroundColor=co;}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS();
				if (Mode==1) rPS.w=per*srcPS.w,rPS.h=per*srcPS.h;
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				//DD<<"PW: "<<rPS.w<<" "<<rPS.h<<"\n";
			}
			
			void SetPic(SDL_Texture *_src,int _mode,bool DeleteLast)
			{
				if (DeleteLast) SDL_DestroyTexture(src);
				src=_src;Mode=_mode;//Full=_full;
				per=1;
				srcPS=GetTexturePosize(src);
				//SDL_QueryTexture(src,NULL,NULL,&srcPS.w,&srcPS.h);
				if (Mode==0) 	;
				else if (Mode==1) CalcPsEx();
				else if (Mode==2)	;
			//	if (!Full) srcPS=srcPS&MakePosize(0,0,rPS.w,rPS.h);
			}
			
			void SetClickFunc(void(*_func)(void*,int),void *_funcData)
			{
				func=_func;
				if (_funcData==CONST_THIS) funcData=this;
				else funcData=_funcData;
			}
			
//			void ResizeToPic()
//			{
//				SDL_QueryTexture(src,NULL,NULL,&srcPS.w,&srcPS.h);
//				rPS=srcPS;
//				CalcPsEx();
//			}
			
			void ExpandShrinkPic(bool setdelta,double _per)
			{
				if (Mode==1)
				{
					if (setdelta) per+=_per;
					else per=_per;
					CalcPsEx();
				}
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (func==NULL) return;
				if (event.type==SDL_MOUSEBUTTONDOWN&&stat==0)
				{
					DD<<"Button Down In PicWg "<<ID<<"\n";
					if (gPS.In(pt))
						if (event.button.button==SDL_BUTTON_LEFT)
							stat=1,MouseEventFlag=-1;
						else if (event.button.button==SDL_BUTTON_RIGHT)
							stat=2,MouseEventFlag=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat!=0)
				{
					DD<<"Button Up In PicWg "<<ID<<"\n";
					if (gPS.In(pt))
						func(funcData,stat),stat=0;
					NeedFreshScreenFlag=1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (func==NULL) return; 
				if (stat!=0)
					if (!lmt.In(pt))
						stat=0;
			}
			
			virtual void Show(Posize &lmt)
			{
				RenderFillRect(lmt=gPS&lmt,BackGroundColor);
//				if (Full)
//				{
//					double fx=srcPS.w*1.0/gPS.w,fy=srcPS.h*1.0/gPS.h;
//					RenderCopy(src,MakePosize((lmt.x-gPS.x)*fx,(lmt.y-gPS.y)*fy,lmt.w*fx,lmt.h*fy),lmt);
//				}
//				else RenderCopy(src,srcPS&(lmt-gPS),(srcPS+gPS)&lmt);
				if (Mode==0)
					RenderCopyWithLmt(src,ZERO_POSIZE,gPS,lmt);
				else if (Mode==1)
					RenderCopy(src,srcPS&(lmt-gPS),(srcPS+gPS)&lmt);

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			~PictureWg()
			{
				DD<<"Delete PictureWg "<<ID<<"\n";
				SDL_DestroyTexture(src);
			}
					
			PictureWg(int _ID,bool _visible,Widgets *_fa,Posize _rPS,SDL_Texture *_tex,int _mode)
			{
				DD<<"Create PictureWg "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
		//		src=_tex;Mode=_mode;//Full=_full;
				SetPic(_tex,_mode,0);
				//SDL_QueryTexture(src,NULL,NULL,&srcPS.w,&srcPS.h);
				//if (!Full) srcPS=srcPS&MakePosize(0,0,rPS.w,rPS.h);
			}
	};
	
	class Slider:public Widgets
	{
		public:
			#define Slider_DefaultBarWidth 3
			#define Slider_DefaultChunkWidth 8
			bool Vertical;
			Posize ckPs,barPs;
			double Percent=0;
			int stat=0;//0:Up_NoFocus 1:Up_Focus_Chunk 2:Down_Slide_Chunk(OccupyPos)
			void (*func)(void*,double,bool)=NULL;
			void *funcData=NULL;
			double (*UpdateCkPsFunc)(void)=NULL;//??
			RGBA BarColor={0,0,0,20},
				 ChunkColor[3]={{0,100,255,127},{0,100,255,160},{0,100,255,255}};
			
			void SetBarColor(RGBA co)
			{BarColor=co;}
			
			void SetChunkColor(int p,RGBA co)
			{ChunkColor[p]=co;}

			void SetPercent(double per)
			{
				Percent=EnsureInRange(per,0,1);
				if (Vertical) ckPs.y=(barPs.h-ckPs.h)*Percent;//??
				else ckPs.x=(barPs.w-ckPs.w)*Percent;
			}
			
			void SetPercentWithFunc(double per)
			{
				SetPercent(per);
				if (func!=NULL)
					func(funcData,Percent,1);
			}
			
			void SetPercentWithFunc_Delta(double per)
			{SetPercentWithFunc(Percent+per);}
			
			void SetChunkPs(Point pt,bool bo)
			{
				if (Vertical)
				{
					ckPs.y=EnsureInRange(pt.y-gPS.y-ckPs.h/2,0,barPs.h-ckPs.h);
					Percent=ckPs.y*1.0/(barPs.h-ckPs.h);
					if (func!=NULL)
						func(funcData,ckPs.y*1.0/(barPs.h-ckPs.h),bo);
				}
				else 
				{
					ckPs.x=EnsureInRange(pt.x-gPS.x-ckPs.w/2,0,barPs.w-ckPs.w);
					Percent=ckPs.x*1.0/(barPs.w-ckPs.w);
					if (func!=NULL)
						func(funcData,ckPs.x*1.0/(barPs.w-ckPs.w),bo);
				}
			//	DD<<"per3 "<<Percent<<"\n";
			}
			
			void SetUpdateCkPs(double (*_UpdateCkPsFunc)(void))
			{UpdateCkPsFunc=_UpdateCkPsFunc;}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS();
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				
				if (Vertical)//??
					barPs={rPS.w/2-1,0,Slider_DefaultBarWidth,rPS.h},
					ckPs={0,0,rPS.w,Slider_DefaultChunkWidth};
				else 
					barPs={0,rPS.h/2-1,rPS.w,Slider_DefaultBarWidth},
					ckPs={0,0,Slider_DefaultChunkWidth,rPS.h};
				SetPercent(Percent);//??
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt))
						{
							DD<<"Sl.Down "<<ID<<"\n";
							stat=2,MouseEventFlag=-1;
							OccupyPosState=1;
							OccupyPosWd=this;
							SetChunkPs(pt,0);
							NeedFreshScreenFlag=1;
						}
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat==2)
				{
					DD<<"Sl.Up "<<ID<<"\n";
					//if (gPS.In(pt))
					{
						stat=1;
						OccupyPosState=0;
						OccupyPosWd=NULL;
						SetChunkPs(pt,1);
					}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==0)
				{
					DD<<"Sl.Motion "<<ID<<"\n";
					stat=1;
					MouseEventFlag=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==2)
				{
					DD<<"Sl.Move "<<ID<<"\n";
					MouseEventFlag=-1;
					SetChunkPs(pt,0);
					NeedFreshScreenFlag=1;
				}
			//	DD<<"per2 "<<Percent<<"\n";
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat!=0)
					if (!lmt.In(pt))
						stat=0,NeedFreshScreenFlag=1;
			}
			
			virtual void Show(Posize &lmt)
			{
				if (UpdateCkPsFunc!=NULL&&stat!=2)
				{
					double UpdateCkPsFuncRet=UpdateCkPsFunc();
					if (UpdateCkPsFuncRet>=0)
						SetPercent(UpdateCkPsFuncRet);
				}
			//	DD<<"per1 "<<Percent<<"\n";
				
				RenderFillRect((barPs+gPS)&lmt,BarColor);
				RenderFillRect((ckPs+gPS)&lmt,ChunkColor[stat]); 
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS),
					Debug_DisplayBorder(ckPs+gPS),
					Debug_DisplayBorder(barPs+gPS);
			}
			
			~Slider()
			{
				DD<<"Delete Slider Custom "<<ID<<"\n";
				if (stat==2)
				{
					OccupyPosState=0;
					OccupyPosWd=NULL;
				}
			}
			
			Slider(int _ID,bool _visible,Widgets *_fa,Posize _rPS,bool _ver,void (* _func)(void*,double,bool),void *_funcdata)
			{
				DD<<"Create Slider Custom "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				Vertical=_ver;
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				func=_func;
				if (_funcdata==CONST_THIS) funcData=this;
				else funcData=_funcdata;
				if (Vertical)
					barPs={rPS.w/2-1,0,Slider_DefaultBarWidth,rPS.h},
					ckPs={0,0,rPS.w,Slider_DefaultChunkWidth};
				else 
					barPs={0,rPS.h/2-1,rPS.w,Slider_DefaultBarWidth},
					ckPs={0,0,Slider_DefaultChunkWidth,rPS.h};
			}
	};
	
	struct MenuData//使用Union如何？ 
	{
		int type=0;//0:none 1:normal_func 2:divide_line 3:sub_menu(user funcdata storage the next *menudata)
		void (*func)(void*)=NULL;
		void *funcdata=NULL;
		int HotKey=0;
		SDL_Texture *pic=NULL;
		bool enable=1;
		string text;
	};
	
	MenuData MakeMenudata_Normal(void (*_func)(void*),void *_funcdata,int _HotKey,SDL_Texture *_pic,bool _enable,string _text)
	{
		MenuData re;
		re.type=1;
		re.func=_func;	re.funcdata=_funcdata;	re.HotKey=_HotKey;
		re.pic=_pic;	re.enable=_enable; 		re.text=_text;
		return re;
	}
	
	MenuData MakeMenudata_DivideLine()
	{
		MenuData re;
		re.type=2;
		return re;
	}
	
	MenuData MakeMenudata_SubMenu(vector <MenuData> *submenu,int _HotKey,SDL_Texture *_pic,bool _enable,string _text)
	{
		MenuData re;
		re.type=3;		re.funcdata=submenu;	re.HotKey=_HotKey;
		re.pic=_pic;	re.enable=_enable; 		re.text=_text;
		return re;
	}
	
	class Menu1:public Widgets
	{
		public:
			vector <MenuData> *Menu1Data;
			//MenuData *Menu1Data=NULL;
			int stat=0,//0:Up_NoFocus 1:Up_Focus 2:Down
				spos1=0,spos2=0,
				pos=-1,//-1:no pos -2:ExceedTop -3:ExceedButtom
				subMenu1pos=-1,
				EachHeight=20,
				DivideLineHeight=3,
				BorderWidth=4,
				MenudataCnt=0;
			bool Exceed=0,
				 SubMenu1=0;
			void (*autoDeleteDataFunc)(void*,vector <MenuData> *)=NULL;
			void *autoDelDataFuncData=NULL;
			RGBA TextColor[2]={RGBA_BLACK,RGBA_GRAY_8[4]},//enable,disable
				 DivideLineColor=RGBA_BLUE_8[2],
				 BackgroundColor[3]={{248,248,248,255},RGBA_BLUE_8/*A*/[4],RGBA_BLUE_8/*A*/[7]};//nofocus(realBG),focus,down

			void SetTextColor(int p,RGBA co)
			{TextColor[p]=co;}
			
			void SetBackgroundColor(int p,RGBA co)
			{BackgroundColor[p]=co;}
			
			int GetHFromData()
			{
				int re=BorderWidth*2;
				if (Menu1Data!=NULL)
					for (int i=0;i<MenudataCnt;++i)//??
						switch ((*Menu1Data)[i].type)
						{
							case 1: case 3:
								re+=EachHeight;
								break;
							case 2:
								re+=DivideLineHeight;
								break;
						}
				return re;
			}

			void SetMenuData(vector <MenuData> *data)
			{
				if (data==NULL) return;
				if (autoDeleteDataFunc!=NULL)
				{
					autoDeleteDataFunc(autoDelDataFuncData,Menu1Data);
					autoDelDataFuncData=NULL;
					autoDeleteDataFunc=NULL;
				}
				Menu1Data=data;
				if (Menu1Data!=NULL)
					MenudataCnt=Menu1Data->size();
				rPS.h=GetHFromData();
			}
			
			void SetAutoDeleteDataFunc(void (*_autoDeleteDataFunc)(void*,vector <MenuData> *),void *_autoDelDataFuncData)
			{
				autoDeleteDataFunc=_autoDeleteDataFunc;
				autoDelDataFuncData=_autoDelDataFuncData;
			}
			
			int GetPos(int y)
			{
				if (!InRange(y,BorderWidth,rPS.h-BorderWidth-1)) return -1;
				if (Exceed)
				{
					if (InRange(y-BorderWidth,0,EachHeight-1))
						return -2;
					if (InRange(rPS.h-y-1,0,EachHeight-1))
						return -3;
					y-=EachHeight+BorderWidth;
				}
				else y-=BorderWidth;
				
				for (int i=spos1;i<MenudataCnt&&y>=BorderWidth&&i<=spos2;++i)
				{
					switch ((*Menu1Data)[i].type)
					{
						case 1: case 3:
							if (InRange(y,0,EachHeight-1))
								return i;
							y-=EachHeight;
							break;
						case 2:
							if (InRange(y,0,DivideLineHeight-1))
								return i;
							y-=DivideLineHeight;
							break;
					}
				}
				return -1;
			}
			
			void SetDelSubMenu1()
			{
				if (!InRange(pos,0,MenudataCnt-1))
				{
					if (subMenu1pos!=-1)
					{
						delete childWg;
						subMenu1pos=-1;
					}
					return;
				}
				
				if (subMenu1pos==-1||pos!=subMenu1pos)
				{
					if ((*Menu1Data)[pos].type==3&&(*Menu1Data)[pos].enable)
					{
						if (subMenu1pos!=-1)
						{
							delete childWg;
							subMenu1pos=-1;
						}
						if (childWg==NULL)
						{
							Point pt;
							pt.x=rPS.w-2*BorderWidth;
							pt.y=BorderWidth;
							if (Exceed)
								pt.y+=EachHeight;
							for (int i=spos1;i<pos;++i)
							{
								switch ((*Menu1Data)[i].type)
								{
									case 1: case 3: pt.y+=EachHeight; 		 break;
									case 2: 		pt.y+=DivideLineHeight; break;
								}
							}
							new Menu1(ID*100000+1,this,pt,rPS.w,(vector <MenuData> *)((*Menu1Data)[pos].funcdata));
							subMenu1pos=pos;
						}
					}
					else if (subMenu1pos!=-1)
					{
						delete childWg;
						subMenu1pos=-1;
					}
				}
			}
			
			virtual void Check(SDL_Event &event)
			{
				if (event.type==SDL_KEYDOWN)
				{
					switch (event.key.keysym.sym)
					{
						
					}
				}
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (gPS.In(pt))
					MouseEventFlag=-1;
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"Menu1 Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.Shrink(BorderWidth).In(pt))
						{
							stat=2;
							pos=GetPos(pt.y-gPS.y);
							NeedFreshScreenFlag=1;
						}
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat==2)
				{
					DD<<"Menu1 Up "<<ID<<"\n";
					if (gPS.Shrink(BorderWidth).In(pt))
					{
						pos=GetPos(pt.y-gPS.y);
						SetDelSubMenu1();
						stat=1;
						NeedFreshScreenFlag=1;
						if (InRange(pos,0,MenudataCnt-1))
							if ((*Menu1Data)[pos].type==1&&(*Menu1Data)[pos].enable)
							{
								if ((*Menu1Data)[pos].func!=NULL)
									(*Menu1Data)[pos].func((*Menu1Data)[pos].funcdata);
								delete MenusLayer->childWg;
							}
					}
				}
				else if (event.type==SDL_MOUSEMOTION)
				{
					if (gPS.Shrink(BorderWidth).In(pt))
					{
						if (stat!=2)
							stat=1;
						int pos_r=pos;
						pos=GetPos(pt.y-gPS.y);
						if (pos!=pos_r)
						{
							SetDelSubMenu1();
							NeedFreshScreenFlag=1;
							DD<<"Menu1 Focus "<<ID<<"\n";
						}
					}
				}
			}

			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				//lmt=lmt&gPS;
				if (!(lmt&gPS).In(pt))
				{
					//if (!SubMenu1)
						if (PUI_NowSolvingEvent->type==SDL_MOUSEBUTTONDOWN)
						{
							Widgets *i=this;
							for (;i!=NULL;i=i->childWg)
								if ((lmt&i->gPS).In(pt))
									break;
							if (i==NULL)
								delete this;
						}
					if (stat==1)
						stat=0,pos=-1,NeedFreshScreenFlag=1,DD<<"Menu1 LoseFocus "<<ID<<"\n";
				}
			}
			
			virtual void Show(Posize &lmt)
			{
//				DD<<"Menu1 "<<ID<<" Show\n";
//				lmt.PrintPosize();
//				gPS.PrintPosize();
				RenderFillRect(gPS.Shrink(1)&lmt,BackgroundColor[0]);
				RenderDrawRectWithLimit(gPS,BackgroundColor[1],lmt);
				int y=BorderWidth;
				if (Exceed)
				{
					if (pos==-2&&stat!=0)
						RenderFillRect((MakePosize(BorderWidth,y,rPS.w-BorderWidth*2,EachHeight)+gPS)&lmt,BackgroundColor[stat]);
					SDL_Texture *tex=CreateTextureFromSurfaceAndDelete(CreateTriangleSurface(EachHeight*2+1,EachHeight,{0,0},{EachHeight*2,0},{EachHeight,EachHeight-1},RGBA_WHITE));
					RenderCopyWithLmt(tex,ZERO_POSIZE,MakePoint(((rPS.w-EachHeight*2-1)>>1)+BorderWidth,y)+gPS.GetLU(),lmt);
					SDL_DestroyTexture(tex);
					y+=EachHeight;
				}
				
//	struct MenuData
//	{
//		int type=0;//0:none 1:normal_func 2:divide_line 3:sub_menu(user funcdata storage the next *menudata)
//		void (*func)(void*)=NULL;
//		void *funcdata=NULL;
//		int HotKey=0;
//		SDL_Texture *pic=NULL;
//		bool enable=1;
//		string text;
//		
//	};
				
				for (int i=spos1;i<MenudataCnt;++i)
				{
					if (y+EachHeight>rPS.h-BorderWidth)
						break;
					switch ((*Menu1Data)[i].type)
					{
						case 1:
							if (stat!=0&&pos==i)
								RenderFillRect((MakePosize(BorderWidth,y,rPS.w-BorderWidth*2,EachHeight)+gPS)&lmt,BackgroundColor[stat]);
							if ((*Menu1Data)[i].pic!=NULL)
								RenderCopyWithLmt((*Menu1Data)[i].pic,ZERO_POSIZE,MakePosize(BorderWidth,y,EachHeight,EachHeight)+gPS.GetLU(),lmt);
							RenderDrawTextWithLmt((*Menu1Data)[i].text,MakePosize(EachHeight+BorderWidth+5,y,gPS.w-BorderWidth*2-EachHeight,EachHeight)+gPS,-1,TextColor[!(*Menu1Data)[i].enable],lmt);
							y+=EachHeight;
							break;
						case 2:
							RenderFillRect((MakePosize(BorderWidth,y,rPS.w-BorderWidth*2,DivideLineHeight)+gPS)&lmt,stat!=0&&pos==i?BackgroundColor[stat]:DivideLineColor);
							y+=DivideLineHeight;
							break;
						case 3:
							if (stat!=0&&pos==i)
								RenderFillRect((MakePosize(BorderWidth,y,rPS.w-BorderWidth*2,EachHeight)+gPS)&lmt,BackgroundColor[stat]);
							if ((*Menu1Data)[i].pic!=NULL)
								RenderCopyWithLmt((*Menu1Data)[i].pic,ZERO_POSIZE,MakePosize(BorderWidth,y,EachHeight,EachHeight)+gPS.GetLU(),lmt);
							RenderDrawTextWithLmt((*Menu1Data)[i].text,MakePosize(EachHeight+BorderWidth+5,y,gPS.w-BorderWidth*2-EachHeight,EachHeight)+gPS,-1,TextColor[!(*Menu1Data)[i].enable],lmt);
							SDL_Texture *tex=CreateTextureFromSurfaceAndDelete(CreateTriangleSurface(EachHeight/2,EachHeight/2,{0,0},{0,EachHeight/2-1},{EachHeight/4-1,EachHeight/4},RGBA_BLACK));
							RenderCopyWithLmt(tex,ZERO_POSIZE,MakePosize(rPS.w-BorderWidth*2-EachHeight/4,y+EachHeight/4,EachHeight/2,EachHeight/2)+gPS.GetLU(),lmt);
							SDL_DestroyTexture(tex);
							y+=EachHeight;
							break;
					}
					spos2=i;
				}
				
				if (Exceed)
				{
					
				}

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			virtual void _SolvePosEvent(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (MouseEventFlag!=-1)
					if (visible)
					{
						if (childWg!=NULL) childWg->_SolvePosEvent(event,pt,MouseEventFlag);
						if (gPS.In(pt))
							if (MouseEventFlag!=-1)
								CheckPos(event,pt,MouseEventFlag);
					}
				if (brother!=NULL) brother->_SolvePosEvent(event,pt,MouseEventFlag);
			}
			
//			virtual void _PresentWidgets(Posize lmt)
//			{
//				if (brother!=NULL) brother->_PresentWidgets(lmt);
//				if (!visible) return;
//				Show(lmt);
//				if (childWg!=NULL) childWg->_PresentWidgets(lmt);
//			}
			
			virtual ~Menu1()
			{
				DD<<"Delete Menu1 "<<ID<<"\n";
				if (autoDeleteDataFunc!=NULL)
				{
					autoDeleteDataFunc(autoDelDataFuncData,Menu1Data);
					autoDelDataFuncData=NULL;
					autoDeleteDataFunc=NULL;
				}
				if (SubMenu1)
					((Menu1*)fa)->subMenu1pos=-1;
			}
			
			Menu1(int _ID,Menu1 *_faMenu1,Point pt,int w,vector <MenuData> *menudata)
			{
				DD<<"Create Menu1 "<<_ID<<"\n";
				SetID(_ID);visible=1;
				BuildRelation(_faMenu1,this);
				SetMenuData(menudata);
				rPS={pt.x,pt.y,w,rPS.h};
				CalcPsEx();
				SubMenu1=1;
			}
			
			Menu1(int _ID,int w,vector <MenuData> *menudata)
			{
				DD<<"Create Menu1 "<<_ID<<"\n";
				SetID(_ID);visible=1;
				BuildRelation(MenusLayer,this);
				SetMenuData(menudata);
				rPS={NowPos.x,NowPos.y,w,rPS.h};
				CalcPsEx();
				SubMenu1=0;
			}
	};
	
	class AddressSection:public Widgets
	{
		public:
			struct AddressSectionData
			{
				//string str;
				PathWithType path;//
				SDL_Texture *tex=NULL;
				Posize texrPS;
				bool Display;
			};
			//  |<|XXX|>|YYYYYYYYY|>|ZZZZ|
			
			int stat=0;//0:Up_NoFocus 1:Up_Focus i 2:Down i
			void (*func)(void*,AddressSection*,PathWithType,int)=NULL;//funcdata,this,focus pwt,NowFocus
			void *funcData=NULL;
//			vector <PathWithType> (*JointFunc)(void*,)
//			void *JointFuncdata=NULL;
			vector <AddressSectionData> AddrSecData_V;
			int SectionCnt=0,
				JointW=14,
				FirstDisplay=-1,
				NowFocus=0;//0:not in +i:in i -i:in i list     //Do remember +-1 when using AddrSecData_V
			bool Updated=0;
			RGBA BackgroundColor[6]={RGBA_BLUE_8[2],RGBA_BLUE_8[1],RGBA_BLUE_8[4],RGBA_BLUE_8[3],RGBA_BLUE_8[6],RGBA_BLUE_8[5]},//0:no_focus_Section 1:no_focus_Joint 2:focus_Section 3:focus_Joint 4:down_Section 5:down_Joint
				 JointTriangleColor[3]={{240,240,240,255},{230,230,230,255},{220,220,220,255}},//0:no_focus_Joint 1:focus_Joint 2:down_Joint
				 TextColor=RGBA_BLACK;
			
			void SetTextColor(RGBA co)
			{TextColor=co;}
			
			void SetBackgroundColor(int p,RGBA co)
			{BackgroundColor[p]=co;}
			
			void SetJointTriangleColor(int p,RGBA co)
			{JointTriangleColor[p]=co;}
			
			void UpdateDisplayPart()//Must run after changing
			{
				if (Updated) return;
				int wSum=0;
				for (int i=SectionCnt;i>=1;--i)
				{
					wSum+=AddrSecData_V[i-1].texrPS.w+JointW;
					if (wSum<=rPS.w||i==SectionCnt)
					{
						AddrSecData_V[i-1].Display=1;
						AddrSecData_V[i-1].texrPS.y=(rPS.h-AddrSecData_V[i-1].texrPS.h)>>1;
						FirstDisplay=i;
					}
					else AddrSecData_V[i-1].Display=0;
				}
				wSum=0;
				if (FirstDisplay!=-1)
					for (int i=FirstDisplay;i<=SectionCnt;++i)
					{
						AddrSecData_V[i-1].texrPS.x=wSum+JointW;
						wSum+=JointW+AddrSecData_V[i-1].texrPS.w;
					}
				Updated=1;
			}
			
			void ClearSection()
			{
				SectionCnt=0;
				for (vector <AddressSectionData>::iterator p=AddrSecData_V.begin();p!=AddrSecData_V.end();++p)
					if ((*p).tex!=NULL)
						SDL_DestroyTexture((*p).tex);
				AddrSecData_V.clear();
				Updated=0;
			}
			
			void AddSectionBack(PathWithType pa)
			{
				AddressSectionData addrdata;
				addrdata.path=pa;
				addrdata.tex=CreateRGBATextTexture(pa.appearance.c_str(),TextColor);
				addrdata.texrPS=GetTexturePosize(addrdata.tex);
				++SectionCnt;
				AddrSecData_V.push_back(addrdata);
				Updated=0;
			}
			
			void DeleteSectionBack()
			{
				if (SectionCnt==0) return;
				SDL_DestroyTexture(AddrSecData_V[--SectionCnt].tex);
				AddrSecData_V.erase(AddrSecData_V.begin()+SectionCnt);
				Updated=0;
			}
			
			int GetChosenSection(int x)//0:not in +i:in i -i:in i list
			{
				for (int i=FirstDisplay;i<=SectionCnt;++i)
				{
					if (InRange(x,AddrSecData_V[i-1].texrPS.x-JointW,AddrSecData_V[i-1].texrPS.x-1))
						return -i;
					if (InRange(x,AddrSecData_V[i-1].texrPS.x,AddrSecData_V[i-1].texrPS.x2()))
						return i;
				}
				return 0;
			}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS();
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				Updated=0;
				UpdateDisplayPart();
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (SectionCnt==0||!Updated) return;
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"AddressSection Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
					{
						if (gPS.In(pt))
						{
							stat=2;
							MouseEventFlag=-1;
							NeedFreshScreenFlag=1;
							NowFocus=GetChosenSection((pt-gPS.GetLU()).x);
						}
					}	
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat==2)
				{
					DD<<"AddressSection Up "<<ID<<"\n";
					if (gPS.In(pt))
					{
						int nNowFocus;
						if (NowFocus!=0&&(nNowFocus=GetChosenSection((pt-gPS.GetLU()).x))==NowFocus)
						{
							if (NowFocus>0) 
							{
								if (func!=NULL)
									func(funcData,this,AddrSecData_V[NowFocus-1].path,NowFocus);
								while (SectionCnt>NowFocus)
									DeleteSectionBack();
								UpdateDisplayPart();
							}	
							else
							{
								DD<<"Solve Joint event here\n";
							}
						}
						else NowFocus=nNowFocus;
						stat=1;
						NeedFreshScreenFlag=1;
					}
				}
				else if (event.type==SDL_MOUSEMOTION&&stat!=2)
				{
					if (gPS.In(pt))
					{
						int nNowFocus;
						if ((nNowFocus=GetChosenSection((pt-gPS.GetLU()).x))!=NowFocus)
						{
							DD<<"AddressSection Focus "<<ID<<"\n";
							NowFocus=nNowFocus;
							stat=1;
							NeedFreshScreenFlag=1;
						}
					}
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				if (SectionCnt==0||!Updated) return;
				lmt=lmt&gPS;
				if (stat!=0)
					if (!lmt.In(pt))
						stat=0,NowFocus=0,NeedFreshScreenFlag=1,DD<<"AddressSection LoseFocus "<<ID<<"\n";
			}
			
			virtual void Show(Posize &lmt)
			{
				lmt=lmt&gPS;
				if (SectionCnt!=0&&Updated)
				{
					Posize tmpPS;
					for (int i=FirstDisplay;i<=SectionCnt;++i)
					{
						tmpPS={gPS.x+AddrSecData_V[i-1].texrPS.x-JointW,gPS.y,JointW,gPS.h};
						//if (i==1)
						{
							RenderFillRect(tmpPS&lmt,BackgroundColor[stat!=0&&(-NowFocus)==i?(stat==1?3:5):1]);
							SDL_Texture *tex=CreateTextureFromSurfaceAndDelete(FirstDisplay==i
											?CreateTriangleSurface(JointW,gPS.h,{3,4},{3,gPS.h-4},{JointW-3,gPS.h>>1},JointTriangleColor[stat!=0&&(-NowFocus)==i?stat:0])
											:CreateTriangleSurface(JointW,gPS.h,{3,gPS.h>>1},{JointW-3,4},{JointW-3,gPS.h-4},JointTriangleColor[stat!=0&&(-NowFocus)==i?stat:0]));
							RenderCopyWithLmt(tex,ZERO_POSIZE,tmpPS,lmt);
							SDL_DestroyTexture(tex);
						}
						//else
						//{
						//	
						//}
						
						tmpPS=AddrSecData_V[i-1].texrPS+gPS;
						tmpPS.h=rPS.h;tmpPS.y=gPS.y;
						RenderFillRect(tmpPS&lmt,BackgroundColor[stat!=0&&NowFocus==i?(stat==1?2:4):0]);
						
						RenderCopyWithLmt(AddrSecData_V[i-1].tex,ZERO_POSIZE,AddrSecData_V[i-1].texrPS+gPS,lmt);
					}
				}

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			~AddressSection()
			{
				DD<<"Delete AddressSection "<<ID<<"\n";
				ClearSection();
			}
					
			AddressSection(int _ID,bool _visible,Widgets *_fa,Posize _rPS,void(*_func)(void*,AddressSection*,PathWithType,int),void *_funcData)
			{
				DD<<"Create AddressSection "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;CalcPsEx();
				func=_func;
				if (_funcData==CONST_THIS) funcData=this;
				else funcData=_funcData;
			}
	};
	
	class TextEditLine:public Widgets//The most complex widgets before 19.4.4
	{
		public:
			//question: #Move view part;#key event
			wstring text;
			string editingText;
			vector <int> ChWidth;
			int stat=0,//0:no focus 1:focus 2:down(occupy pos)
				pos1=-1,pos2=-1,
				spos1=0,spos2=0,
				SumWidth=0,
				ChHeight=0,
				LenLmt=-1,
				EditingTextCursorPos,
				LastPos2X=0,//It is so ugly(but effective); 
				LastEditingTimeStamp=0;
			Uint32 UpdateSposInterval=100;
			bool ShowBorder=1,
				 StateInput=0,
				 Editing=0,
				 IntervalTimerOn=0;
//			atomic_bool EnableIntervalTimer;
			SDL_TimerID IntervalTimerID;
			void (*EnterFunc)(void*,string)=NULL;
			void *funcData=NULL;
			RGBA TextColor[3]={RGBA_BLACK,RGBA_WHITE,{0,100,255,200}},//normal,selected,editing 
				 BackgroundColor[4]={{255,255,255,200},{0,100,255,127},{0,100,255,160},{0,100,255,255}},//bg,choosepart stat0,1,2
				 BorderColor[4]={{0,100,255,127},{0,100,255,160},{0,100,255,255},{255,0,0,200}};//stat0,1,2 overlimit
			
			void SetTextColor(int p,RGBA co)
			{TextColor[p]=co;}
			
			void SetBackgroundColor(int p,RGBA co)
			{BackgroundColor[p]=co;}
			
			void SetBorderColor(int p,RGBA co)
			{BorderColor[p]=co;}
			
			void SetLengthLimit(int lenlmt)//cannot be used yet 
			{
				LenLmt=lenlmt;
				
			}
			
			void SetEnterFunc(void (*_EnterFunc)(void*,string),void *_funcData)
			{
				EnterFunc=_EnterFunc;
				funcData=_funcData;
			}
			
			void SetSposFromAnother(bool fromSpos1)
			{
				if (fromSpos1)
				{
					int i,s;
					for (i=spos1,s=3;i<text.length()&&s<=rPS.w-3;++i)
						s+=ChWidth[i];
//					if (i==text.length()) spos2=i;
//					else spos2=i-1;
					if (s<=rPS.w-3) spos2=text.length();
					else spos2=i-1;
				}
				else
				{
					int i,s;
					for (i=spos2-1,s=rPS.w-3;i>=0&&s>=3;--i)
						s-=ChWidth[i];
//					if (i==0) spos1=0;
//					else spos1=i+1;
					if (s>=3) spos1=0;
					else spos1=i+2;
				}
			}
		
			void SetSposFromPos2()
			{
				if (pos2<spos1)
				{
					spos1=pos2;
					SetSposFromAnother(1);
					NeedFreshScreenFlag=1;
				}
				else if (pos2>spos2)
				{
					spos2=pos2;//??
					SetSposFromAnother(0);
					NeedFreshScreenFlag=1;
				}
			}
			
			void SetText(string str)
			{
				SumWidth=0;
				ChWidth.clear();
				LenLmt=-1;
				text=CharSet_Net::Utf8ToUnicode(str);
				GetRidOfEndChar0(text);
				for (int i=0,w=0;i<text.length();++i)
				{
					wstring wstr;
					wstr+=text[i];
					TTF_SizeUTF8(PUI_Font,CharSet_Net::UnicodeToUtf8(wstr).c_str(),&w,NULL);
					ChWidth.push_back(w);
					SumWidth+=w;
				}
				TTF_SizeUTF8(PUI_Font,CharSet_Net::UnicodeToUtf8(text).c_str(),NULL,&ChHeight);
				pos2=pos1=text.length();
				//SetSposFromPos2();
				if (SumWidth>rPS.w-6)
				{
					spos2=text.length();
					SetSposFromAnother(0);
				}
				else 
				{
					spos1=0;
					SetSposFromAnother(1);
				}
			}
			
			void AddText(int p,string str)
			{
				if (p==-1)
				{
					int i=text.length();
					text+=CharSet_Net::Utf8ToUnicode(str);
					text.erase(text.length()-1,1);
					for (int w=0;i<text.length();++i)
					{
						wstring wstr;
						wstr+=text[i];
						TTF_SizeUTF8(PUI_Font,CharSet_Net::UnicodeToUtf8(wstr).c_str(),&w,NULL);
						ChWidth.push_back(w);
						SumWidth+=w;
					}
					pos1=pos2=text.length();
				}
				else
				{
					if (!InRange(p,0,text.length()))
					{
						DD<<"Error TextEditLine "<<ID<<" AddText out of range\n";
						return;
					}
					wstring wstr=CharSet_Net::Utf8ToUnicode(str);
					GetRidOfEndChar0(wstr);
					text.insert(p,wstr);
					for (int w=0,i=0;i<wstr.length();++i)
					{
						wstring ws;
						ws+=wstr[i];
						TTF_SizeUTF8(PUI_Font,CharSet_Net::UnicodeToUtf8(ws).c_str(),&w,NULL);
						ChWidth.insert(ChWidth.begin()+i+p,w);
						SumWidth+=w;
					}//??
					pos1=pos2=p+wstr.length();
				}
				SetSposFromPos2();
			}
			
			void DeleteText(int p,int len)
			{
				for (int i=p,w=0;i<p+len;++i)
				{
					wstring wstr;
					wstr+=text[i];
					TTF_SizeUTF8(PUI_Font,CharSet_Net::UnicodeToUtf8(wstr).c_str(),&w,NULL);
					SumWidth-=w;
				}
				text.erase(p,len);
				ChWidth.erase(ChWidth.begin()+p,ChWidth.begin()+p+len);
				pos1=pos2=p;
				if (spos2>text.length())
					spos2=text.length();
				SetSposFromPos2();
				if (spos1>=text.length())//19.11.24 fixed a little(maybe)
					spos2=text.length(),
					SetSposFromAnother(0);
			}
			
			void DeleteTextBack()
			{
				if (text.length()==0)
				{
					DD<<"Unable DeleteTextBack\n";
					return;
				}
				DeleteText(text.length()-1,1);
			}
			
			void DeleteTextCursor()
			{
				if (pos1==pos2)
				{
					if (pos1==0) return; 
					DeleteText(pos1-1,1);
				}
				else
				{
					int m=min(pos1,pos2),M=max(pos1,pos2);
					DeleteText(m,M-m);
				}
			}
			
			wstring GetSelectedText()
			{
				return text.substr(pos1,pos2-pos1+1);
			}
			
			wstring GetText()
			{
				return text;
			}
			
			void SetCursorPos(int p)
			{
				pos1=pos2=p;
			}
			
			void TurnOnOffIntervalTimer(bool on)
			{
				if (IntervalTimerOn==on) return;
				if (on)
				{
					PUI_UpdateTimerData *p=new PUI_UpdateTimerData;
					p->cnt=-1;
					p->tar=this;
					IntervalTimerID=SDL_AddTimer(UpdateSposInterval,PUI_UpdateTimer,p);
					IntervalTimerOn=1;
				}
				else//Do remember use it when deconstruct this;
				{
					SDL_RemoveTimer(IntervalTimerID);
					IntervalTimerOn=0;
				}
			}
			
			int GetPos(int x)//return char interval index:  0,ch0,1,ch1,2,ch2,3
			{
				LastPos2X=x;
				if (x<3)
				{
					int s=3,i=0;
					for (vector <int>::iterator vp=ChWidth.begin()+spos1;vp!=ChWidth.begin();--vp)
					{
						if (x>s-*vp/2) return i+spos1;
						s-=*vp;--i;
						if (x>=s) return i+spos1;
					}
					return 0;
				}
				else
				{
					int s=3,i=0;
					for (vector <int>::iterator vp=ChWidth.begin()+spos1;vp!=ChWidth.end();++vp)
					{
						if (x<s+*vp/2) return i+spos1;
						s+=*vp;++i;
						if (x<=s) return i+spos1;
					}
					//DD<<"Focus end\n";
					return text.length();
				}
			}
			
			virtual void Check(SDL_Event &event)
			{
				if (event.type==SDL_KEYDOWN)
				{
					//DD<<"lasteditingtimestamp&&keytimestamp "<<event.key.timestamp<<" "<<LastEditingTimeStamp<<"\n";
					if (StateInput)
						if (!Editing)
							if (event.key.timestamp-LastEditingTimeStamp>0)//?? 
								switch (event.key.keysym.sym)
								{
									case SDLK_BACKSPACE:
										DD<<"TextEditLine DeleteTextBack\n";
										//DeleteTextBack();
										DeleteTextCursor();
										NeedFreshScreenFlag=1;
										break;
									case SDLK_LEFT:
										if (pos2>0)
										{
											pos1=--pos2;
											SetSposFromPos2();
										}
										break;
									case SDLK_RIGHT:
										if (pos2<text.length())
										{
											pos1=++pos2;
											SetSposFromPos2();
										}
										break;
									case SDLK_RETURN:
										if (EnterFunc!=NULL)
											EnterFunc(funcData,CharSet_Net::UnicodeToUtf8(text));//??Have end blank??
										break;
									case SDLK_HOME:
										pos1=pos2=0;
										SetSposFromPos2();
										break;
									case SDLK_END:
										pos1=pos2=text.length();
										SetSposFromPos2();
										break;
									case SDLK_v:
										if (event.key.keysym.mod&KMOD_CTRL)
										{
											string str=SDL_GetClipboardText();
											if (str.empty())
												break;
											if (pos1!=pos2)
												DeleteTextCursor();
											AddText(pos2,str);
											SetSposFromPos2();
										}
										break;
								}
				}
				
				else if (event.type==SDL_USEREVENT)
				{
					if (event.user.type==PUI_EVENT_PRESENT)
					{
						if (event.user.data1==this)
						{
							SetSposFromPos2();
							pos2=GetPos(LastPos2X);
						}
					}
				}
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"TextEditLine Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt))
							if (!Editing) 
							{
								stat=2;MouseEventFlag=-1;
								pos1=pos2=GetPos((pt-gPS.GetLU()).x);
								OccupyPosState=1;
								OccupyPosWd=this;
								DD<<"Start TextEditLine Input\n";
							}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat==2)
				{
					DD<<"TextEditLine Up "<<ID<<"\n";
					//if (gPS.In(pt))
					{
						stat=1;
						OccupyPosState=0;
						OccupyPosWd=NULL;
						StateInput=1;
						KeyboardInputWgState=1;
						KeyboardInputWg=this;
						SDL_StartTextInput();
						SDL_Rect rct=gPS.Expand(200).ToRect();
						SDL_SetTextInputRect(&rct);
						TurnOnOffIntervalTimer(0);
					}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==0)
				{
					DD<<"TextEditLine Focus "<<ID<<"\n";
					if (gPS.In(pt))
						stat=1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==2)
				{
					DD<<"TextEditLine Focus "<<ID<<"\n";
					//if (gPS.In(pt)) 
					if (!Editing)
					{
						pos2=GetPos((pt-gPS.GetLU()).x);
						if (!InRange(LastPos2X,gPS.x+3,gPS.x2()-3))
							TurnOnOffIntervalTimer(1);
						else TurnOnOffIntervalTimer(0);
						NeedFreshScreenFlag=1;
					}
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (!lmt.In(pt))
				{
					if (stat==1)
						stat=0,NeedFreshScreenFlag=1,DD<<"TextEditLine LoseFocus "<<ID<<"\n";
					if (StateInput)
						if (PUI_NowSolvingEvent->type==SDL_MOUSEBUTTONDOWN)
							if (PUI_NowSolvingEvent->button.button==SDL_BUTTON_LEFT)
							{
								StateInput=0;Editing=0;
								NeedFreshScreenFlag=1;
								KeyboardInputWg=NULL;
								KeyboardInputWgState=0;
								editingText.clear();
								SDL_StopTextInput();//have question?? (when other wg also use it,it will??)
								TurnOnOffIntervalTimer(0);
								DD<<"Stop TextEditLine Input\n";
							}
				}
			}
			
			virtual void ReceiveKeyboardInput(SDL_Event &event)
			{
				if (event.type==SDL_TEXTINPUT)
				{
					if (pos1!=pos2)
						DeleteTextCursor();
					AddText(pos2,event.text.text);
				}
				else if (event.type==SDL_TEXTEDITING)
				{
					DD<<"Editing Text start "<<event.edit.start<<" "<<"len "<<event.edit.length<<"\n";
					editingText=event.edit.text;
					EditingTextCursorPos=event.edit.start;
					LastEditingTimeStamp=event.edit.timestamp;
					
					if (pos1!=pos2)
						DeleteTextCursor();
					
				}
				Editing=editingText.length()!=0;
				if (!Editing) DD<<"Not editing\n"; 
				NeedFreshScreenFlag=1;
			}
			
			virtual void Show(Posize &lmt)
			{
				//DD<<spos1<<"###"<<spos2<<"\n";
				int editingTextLen=-text.length();
				if (StateInput&&Editing)
				{
					AddText(pos2,editingText);
					editingTextLen+=text.length();
					SetCursorPos(pos2-editingTextLen+EditingTextCursorPos);
				}
				else editingTextLen=0;
				
//				DD<<"TT "<<text.length()<<" "<<ChWidth.size()<<"\n";
//				DD<<"TEL pos1 "<<pos1<<" pos2 "<<pos2<<" spos1 "<<spos1<<" spos2 "<<spos2<<"\n";
				
				RenderFillRect(lmt=gPS&lmt,BackgroundColor[0]);
				if (ShowBorder) RenderDrawRectWithLimit(gPS,BorderColor[stat],lmt);
				
				int x=3,w=0,m=min(pos1,pos2),M=max(pos1,pos2);
				for (int i=spos1;i<ChWidth.size()&&i<=spos2;++i)
					if (i<m) x+=ChWidth[i];
					else if (i>=M) break;
					else w+=ChWidth[i];
				if (StateInput&&pos1==pos2) RenderFillRect((MakePosize(x,3,1,rPS.h-6)+gPS)&lmt,BackgroundColor[stat+1]);
				RenderFillRect((MakePosize(x,3,w,rPS.h-6)+gPS)&lmt,BackgroundColor[stat+1]);
				
				int i,s;
				for (i=spos1,s=3;i<text.length()&&s+ChWidth[i]<=rPS.w-3;s+=ChWidth[i],++i)
				{
					wstring wstr;wstr+=text[i];
					SDL_Texture *tex=CreateRGBATextTexture(CharSet_Net::UnicodeToUtf8(wstr).c_str(),TextColor[editingTextLen!=0&&InRange(i,pos2-EditingTextCursorPos,pos2-EditingTextCursorPos+editingTextLen-1)?2:InRange(i,m,M-1)]);
					RenderCopyWithLmt(tex,ZERO_POSIZE,/*MakePoint(s+gPS.x,3+gPS.h)*/MakePosize(s,rPS.h-ChHeight>>1,ChWidth[i],ChHeight)+gPS,lmt&gPS.Shrink(3));
					SDL_DestroyTexture(tex);
				}
//				DD<<"s "<<s<<" w "<<rPS.w<<" i "<<i<<"\n";
//				if (i==text.length())
					spos2=i;
//				else spos2=i-1;
				
				if (StateInput&&Editing)
					DeleteText(pos2-EditingTextCursorPos,editingTextLen);

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			~TextEditLine()
			{
				DD<<"Delete TextEditLine "<<ID<<"\n";
				TurnOnOffIntervalTimer(0);
				if (stat==2)
				{
					OccupyPosState=0;
					OccupyPosWd=NULL;
				}
				if (StateInput)
				{
					KeyboardInputWgState=0;
					KeyboardInputWg=NULL;
				}
			}
			
			TextEditLine(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create TextEditLine "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;CalcPsEx();
			}
	};
	
	class BasicTextBox:public Widgets
	{
		public:
			struct TextData
			{
				wchar_t ch;
				SDL_Texture *tex;
				Posize texrPS;
			};
			LargeLayerWithScrollBar *fa2=NULL;//use inner scroll when fa2 is NULL
			int stat=0;//0:notfocus 1:focus 2:down(occupyPos)
			vector <TextData> TextData_V;
			int TextCnt=0;
			int Pos1=0,Pos2=0;
			bool Editable=1,
				 Selectable=1,
				 AutoNextLine=1,
				 ReceiveKey=1;
			RGBA TextColor=RGBA_BLACK,
				 BackgroundColor=RGBA_WHITE;
				 
			void InsertText(int p)//p==-1: in cursor
			{
				
			}
			
			void DeleteText(int p1,int p2)//[p1,p2]
			{
				
			}
			
			~BasicTextBox()
			{
				
			}
			
			BasicTextBox(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create BasicTextBox "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;CalcPsEx();
			}
	};
	
	class AdvancedTextBox:public Widgets
	{
		public:
			struct TextData
			{
				wchar_t ch;
				SDL_Texture *tex;
				Posize texrPS;
				RGBA chColor=RGBA_BLACK,
				 	 bgColor=RGBA_WHITE;
				//size,font
//				bool bold=0,
//					 italic=0;
			};
	};
	
	class RadioButton:public Widgets
	{
		public:
			
	};
	
	class IntNumberSetBox:public Widgets
	{
		public:
			void(*func)(void*,int)=NULL;
			void *funcData=NULL;
			int stat=0,//0:No_Focus 1:Focus 2:Down
				Value=0;
			bool HighLighted=0;
			RGBA BorderColor[2]={{0,100,255,63},{0,100,255,127}},
				 BackGroundColor[2]={RGBA_WHITE,{0,0,255,200}},//Normal,HighLight
				 TextColor[2]={RGBA_BLACK,RGBA_WHITE};
			
			void SetBorderColor(int p,RGBA co)
			{BorderColor[p]=co;}
			
			void SetBackGroundColor(int p,RGBA co)
			{BackGroundColor[p]=co;}
			
			void SetTextColor(int p,RGBA co)
			{TextColor[p]=co;}
			
			void SetValue(int val)
			{Value=val;}
			
			virtual void Check(SDL_Event &event)
			{
				
			}
			 
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat!=0)
					if (!lmt.In(pt))
						stat=0;
			}
			
			virtual void Show(Posize &lmt)
			{
				RenderFillRect(lmt=gPS&lmt,BackGroundColor[HighLighted]);
				

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			IntNumberSetBox(int _ID,bool _visible,Widgets *_fa,Posize _rPS,void(*_func)(void*,int),void *_funcData)
			{
				DD<<"Create IntNumberSetBox "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				func=_func;
				if (_funcData==CONST_THIS) funcData=this;
				else funcData=_funcData;
			}
	};
	
	class DoubleNumberSetBox:public Widgets
	{
		public:
			
	};
	
	class ResizeLine:public Widgets
	{
		public:
			#define ResizeLine_DefaultWidth 5 
			bool Vertical;
			int RangeL=0,RangeR=1e9;
			vector <pair<Widgets*,bool> > target;
			int stat=0;//0:No_Focus 1:Focus 2:Down
			RGBA RLColor[3]={{0,100,255,63},{0,100,255,127},{0,100,255,191}};
			
			void SetRLColor(int p,RGBA co)
			{RLColor[p]=co;}
			
			void AddLayer(Layer *tar,bool p)//Same With Below Function    //Only for Layer
			{
				bool ValidFlag=1;
				if (Vertical)//??
					if (p)
						if (tar->rPS.x<rPS.x) 
							RangeL=max(RangeL,tar->rPS.x);
						else ValidFlag=0;
					else
						if (tar->rPS.x2()>rPS.x2())
							RangeR=min(RangeR,tar->rPS.x2());
						else ValidFlag=0;
				else 
					if (p)
						if (tar->rPS.y<rPS.y)
							RangeL=max(RangeL,tar->rPS.y);
						else ValidFlag=0;
					else 
						if (tar->rPS.y2()>rPS.y2())
							RangeR=min(RangeR,tar->rPS.y2());
						else ValidFlag=0;
						
				if (!ValidFlag) DD<<"ResizeLine AddLayer "<<ID<<" Error: Invalid Pos!\n";
				else 
					if (tar->fa==fa) target.push_back({tar,p});
					else DD<<"ResizeLine AddLayer "<<ID<<" Error: Not Same Father!\n";
			}
			
			void AddLayer(LargeLayerWithScrollBar *tar,bool p)
			{
				bool ValidFlag=1;
				if (Vertical)//??
					if (p)
						if (tar->rPS.x<rPS.x) 
							RangeL=max(RangeL,tar->rPS.x);
						else ValidFlag=0;
					else
						if (tar->rPS.x2()>rPS.x2())
							RangeR=min(RangeR,tar->rPS.x2());
						else ValidFlag=0;
				else 
					if (p)
						if (tar->rPS.y<rPS.y)
							RangeL=max(RangeL,tar->rPS.y);
						else ValidFlag=0;
					else 
						if (tar->rPS.y2()>rPS.y2())
							RangeR=min(RangeR,tar->rPS.y2());
						else ValidFlag=0;
						
				if (!ValidFlag) DD<<"ResizeLine AddLayer "<<ID<<" Error: Invalid Pos!\n";
				else 
					if (tar->fa==fa) target.push_back({tar,p});
					else DD<<"ResizeLine AddLayer "<<ID<<" Error: Not Same Father!\n";
			}
			
			void Update()
			{
				for (vector <pair<Widgets*,bool> >::iterator p=target.begin();p!=target.end();++p)
				{
					DD<<"a\n";
					Posize tar_rPS=p->first->rPS;
					if (Vertical)//ver1: p0:L p1:R   ver0: p0:U p1:D
						if (p->second) tar_rPS.SetX2(rPS.x-1);
						else tar_rPS.SetX_ChangeW(rPS.x2()+1);
					else
						if (p->second) tar_rPS.SetY2(rPS.y-1);
						else tar_rPS.SetY_ChangeH(rPS.y2()+1);
					p->first->ResetrPS(tar_rPS);
				}
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"RL Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt))
						{
							OccupyPosState=1;
							OccupyPosWd=this;
							stat=2,MouseEventFlag=-1;
						}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat==2)
				{
					DD<<"RL Up "<<ID<<"\n";
					//if (gPS.In(pt))
					{
						OccupyPosState=0;
						OccupyPosWd=NULL;
						stat=1;
					}
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==0)
				{
					DD<<"RL Focus "<<ID<<"\n";
					if (gPS.In(pt))
						stat=1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==2)
				{
					DD<<"RL Motion "<<ID<<"\n";
					if (Vertical) rPS.x=EnsureInRange(pt.x,RangeL,RangeR)-1;//??
					else rPS.y=EnsureInRange(pt.y,RangeL,RangeR)-1;
					//CalcPS();
					CalcPsEx();
					Update();
					NeedFreshScreenFlag=1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat==1)
					if (!lmt.In(pt))
						stat=0,NeedFreshScreenFlag=1;
			}
			
			virtual void Show(Posize &lmt)
			{
				//CalcPsEx();//??
				RenderFillRect(lmt=gPS&lmt,RLColor[stat]);
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			~ResizeLine()
			{
				DD<<"Delete ResizeLine "<<ID<<"\n";
				if (stat==2)
				{
					OccupyPosState=0;
					OccupyPosWd=NULL;
				}
			}
			
			ResizeLine(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				if (_rPS.w==0) _rPS.w=ResizeLine_DefaultWidth,Vertical=1;
				else if (_rPS.h==0) _rPS.h=ResizeLine_DefaultWidth,Vertical=0;
				else DD<<"Error ResizeLine "<<ID<<" rPS\n";
				rPS=_rPS;//CalcPS();
				CalcPsEx();
			}
	};
	
	class SimpleTextBox:public Widgets
	{
		public:
			LargeLayerWithScrollBar *fa2=NULL;
			vector <wstring> Text;
//			vector <int> TextW;
			vector <bool> isNextline;
			int EachLineHeight=0,LineInterval=0;
			int LineCnt=1,SumW=0;
			RGBA co=RGBA_BLACK,
				 BgCo={0,0,0,20};
			
			void SetTextColor(RGBA _co)
			{co=_co;}
			
			void SetBackgroundColor(RGBA _co)
			{BgCo=_co;}
			
			void _ClearText()
			{
				if (!LineCnt) return;
				DD<<"Clear SimpleTextBox Text\n";
				Text.clear();
				Text.push_back(L"");
				isNextline.clear();
				SumW=0;LineCnt=1;
				rPS.h=fa2->rPS.h;
				fa2->ResizeLL(-1,rPS.h);
				fa2->SetViewPort(1,0);
				fa2->SetViewPort(2,0);
			}
			
			void ClearText()
			{
				if (!LineCnt) return;
				DD<<"Clear SimpleTextBox Text\n";
				_ClearText();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			void ClearForeHalf()
			{
				
			}
			
			map <Uint16,int> WidthOfWch;
			
			void _AddText(wstring wstr)
			{
				//DD<<"_AddText "<<CharSet_Net::UnicodeToUtf8(wstr)<<"\n";
				for (auto sp:wstr)
					if (sp!=0)
						if (/*DD<<sp<<" ",*/sp==L'\n'||sp==L'\r')
							/*DD<<"A\n",Text.back()+=L'\n',*/++LineCnt,SumW=0,Text.push_back(L""),isNextline.push_back(1);
						else if (sp==L'\t')
						{
							int chw=0;
							Uint16 ch[1]={L' '};
							map <Uint16,int>::iterator mp=WidthOfWch.find(ch[0]);
							if (mp==WidthOfWch.end())
								TTF_SizeUNICODE(PUI_Font,ch,&chw,NULL),
								WidthOfWch[ch[0]]=chw;
							else chw=mp->second;
							SumW+=4*chw;
							Text.back()+=L"    ";
						}
						else
						{
							int chw=0;
							Uint16 ch[1]={sp};
							map <Uint16,int>::iterator mp=WidthOfWch.find(ch[0]);
							if (mp==WidthOfWch.end())
								TTF_SizeUNICODE(PUI_Font,ch,&chw,NULL),
								WidthOfWch[ch[0]]=chw;
							else chw=mp->second;
							//DD<<"chw "<<chw<<"\n";
							if (SumW+chw>rPS.w)
								/*DD<<"B\n",*/++LineCnt,SumW=chw,Text.push_back(L""),isNextline.push_back(0);
							else /*DD<<"C\n",*/SumW+=chw;
							//DD<<"D\n";
							Text.back()+=sp;
							//DD<<"E\n";
						}
				//DD<<"_AddText[OK]\n";
			}
			
			void AddText(wstring wstr)
			{
				_AddText(wstr);
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			void ReCalcText()
			{
				DD<<"SimpleTextBox "<<ID<<" ReCalcText\n";
				static int LastW=rPS.w;
				if (rPS.w!=LastW)
				{
					wstring wstr;
					for (int i=0;i<Text.size()-1;++i)
						wstr+=Text[i]+(isNextline[i]?L"\n":L"");
					wstr+=Text.back();
					_ClearText();
					_AddText(wstr);
					LastW=rPS.w;
				}
			}
			
			virtual void CalcPsEx()
			{
				DD<<"SimpleTextBox "<<ID<<" CalcPsEx\n";
				if (PsEx!=NULL) PsEx->GetrPS();
				if (fa2->Not_H) rPS.w=fa2->rPS.w;
				else rPS.w=fa2->rPS.w-fa2->ScrollBarWidth;
				ReCalcText();
				rPS.h=EnsureInRange(LineCnt*(EachLineHeight+LineInterval)-LineInterval,fa2->rPS.h,1e9);
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				fa2->ResizeLL(rPS.w,rPS.h);
			}
			
			
			virtual void Show(Posize &lmt)
			{
			//	lmt=lmt&gPS;
				DD<<"Show\n";
				if (LineCnt==0) return;
				int ForL=(-fa2->deltaPos.y)/(EachLineHeight+LineInterval),
					ForR=ForL+fa2->gPS.h/(EachLineHeight+LineInterval)+1;
				ForL=EnsureInRange(ForL,0,LineCnt-1);
				ForR=EnsureInRange(ForR,0,LineCnt-1);
				Posize LinePosize=MakePosize(gPS.x,gPS.y+ForL*(EachLineHeight+LineInterval),gPS.w,EachLineHeight),_pps;
				for (int i=ForL;i<=ForR;LinePosize.y+=EachLineHeight+LineInterval,++i)
				{
					//DD<<"Fori "<<i<<"\n";
					if (Text[i]==L"") continue;
					SDL_Surface *sur=CreateRGBATextSurface((CharSet_Net::UnicodeToUtf8(Text[i])).c_str(),co);
					SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
					RenderCopy(tex,(lmt-LinePosize)&MakePosize(0,0,sur->w,sur->h),
								_pps=lmt&MakePosize(LinePosize.x,((LinePosize.h-sur->h)>>1)+LinePosize.y,sur->w,sur->h));	
					if (DEBUG_DisplayBorderFlag)
						Debug_DisplayBorder(_pps);
					SDL_FreeSurface(sur);
					SDL_DestroyTexture(tex);
				}
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			void AddPsEx(PosizeEX *psex)
			{
				if (fa2->PsEx!=NULL)
					psex->nxt=fa2->PsEx;
				fa2->PsEx=psex;
				fa2->PsEx->wg=fa2;
				fa2->CalcPsEx();
				NeedUpdatePosize=1;
			}
			
			~SimpleTextBox()
			{
				DD<<"Delete SimpleTextBox "<<ID<<"\n";
			}
			
			SimpleTextBox(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create SimpleTextBox "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				fa2=new LargeLayerWithScrollBar(ID*100000+1,1,_fa,_rPS,_rPS.w,_rPS.h);
				fa2->SetLayerColor(BgCo);
				BuildRelation(fa2,this);
				rPS={0,0,rPS.w,0};
				Widgets::AddPsEx(new PosizeEX_BindrPS(fa2,PosizeEX_BindrPS::W_CX2));
				EachLineHeight=TTF_FontLineSkip(PUI_Font);
				Text.push_back(L"");
				LineCnt=1;
			}
	};
	
//	class TerminalStyleTextBox:public Widgets
//	{
//		public:
//			vector <wchar_t> TextBuffer;
//			//wchar_t **TextBuffer;
//			//wstring TextBuffer;
//			//vector <int> EachLineHead;
//			void (*readfunc)(void*,wstring)=NULL;
//			void *readfunc_funcdata=NULL;
//			bool Cursor_Type_Chunk=0,
//				 ShowCoordinate=0,
//				 EnableInput=1,
//				 StateInput=0,
//				 StateEditing=0;
//			int MaxBufferSize=1048576,//0: no limit
//				NowShowLine1=0;
//			Point CursorPos;//this is not to say pixel
//			Posize EachCharSize={0,0,7,16};
//			LargeLayerWithScrollBar *fa2=NULL;
//			RGBA BackgroundColor=RGBA_BLACK,
//				 DefaultTextColor=RGBA_GRAY_8[0];
//			
//			void SetMaxBufferSize(int size)
//			{
//				MaxBufferSize=size;
//				TextBuffer.reserve(MaxBufferSize);
//			}
//			
//			void SetCursorPos()
//			{
//				
//			}
//			
//			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
//			{
//				
//			}
//			
//			virtual void Show(Posize &lmt)
//			{
//				
//			}
//			
//			TerminalStyleTextBox(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
//			{
//				DD<<"Create TerminalStyleTextBox "<<_ID<<"\n";
//				SetID(_ID);visible=_visible;
//				fa2=new LargeLayerWithScrollBar(ID*100000+1,1,_fa,_rPS,rPS.w,rPS.h);
//				BuildRelation(fa2,this);
//				rPS=_rPS;
//				CalcPsEx();
//			}
//	};
	
	class TerminalStyleTextBox:public Widgets
	{
		public:
			struct TextData
			{
				wchar_t ch;
				bool Wide=0;
				RGBA co[2];
			};
			vector <TextData> TextBuffer;
			void (*readfunc)(void*,wstring)=NULL;
			void *readfunc_funcdata=NULL;
			bool EnableInput=1,
				 StateInput=0,
				 StateEditing=0;
			int MaxBufferLine=1000,//0: no limit
				Cursor_Type=0,//0:Chunk 1:Vertical_Cursor 2:Horizonal_Cursor
 				NowShowLine1=0,
				EachlineCharCnt=0;
			Point CursorPos;//this is not to say pixel
			Posize EachCharSize={0,0,7,16};
			SDL_Texture *ASCIIcharTex[128];
			Posize ASCIIcharTexSize[128];
			LargeLayerWithScrollBar *fa2=NULL;
			RGBA BackgroundColor=RGBA_BLACK,//Default
				 TextColor=RGBA_GRAY_8[0];
			
			void SetMaxBufferLine(int linecnt)
			{
				MaxBufferLine=linecnt;
				EachlineCharCnt=rPS.w/(EachCharSize.x+EachCharSize.w);
				TextBuffer.reserve(MaxBufferLine*EachlineCharCnt);
			}
			
			void InitASCIIcharTex()
			{
				for (char i=0;i<=127;++i)
					ASCIIcharTex[i]=CreateRGBATextTexture(&i,TextColor),
					ASCIIcharTexSize[i]=GetTexturePosize(ASCIIcharTex[i]);
			}
			
			void SetCursorPos(Point pt)
			{
				
			}
			
			virtual void Check(SDL_Event &event)
			{
				
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				
			}
			
			virtual void Show(Posize &lmt)
			{
				
				
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			TerminalStyleTextBox(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create TerminalStyleTextBox "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
//				fa2=new LargeLayerWithScrollBar(ID*100000+1,1,_fa,_rPS,rPS.w,rPS.h);
//				BuildRelation(fa2,this);
//				rPS=_rPS;
//				CalcPsEx();
			}
	};
	
	class DownDragBox:public Widgets
	{
		public:
			
	};
	
//	class SimpleBlockView:public Widgets
//	{
//		public:
//			int stat=0;//0:Up_NoFocus 1:Up_Focus_Row 2:Down_Left 3:Down_Right 4:LeftClick_Choose_NotFunc
//			int ListCnt=0;
////			vector <string> ListText;
//			vector <void*> FuncData;
//			vector <SDL_Texture*> BlockPic;
//			vector <vector <string> > BlockText;
//			void (*func)(void*,int,int)=NULL;//int1:Pos   int2: 0:None 1:Left_Click 2:Left_Double_Click 3:Right_Click
//			void (*DeleteFunc)(void*)=NULL;
//			LargeLayerWithScrollBar *fa2=NULL;
//			int ChooseWhich1=-1,ChooseWhich2=-1;//1:FocusChoose 2:Click_Choose
//			Posize BlockPosize={5,5,180,70};
////			int EachHeight=20,
////				Interval=2;
////			RGBA TextColor=RGBA_BLACK,
////				 RowColor[3]={{200,200,200,80},{0,100,255,100},{0,100,255,200}};//NoFocusRow,FocusRow,ClickRow
//			RGBA TextColor[2]={{},{}},//Main,Associate
//				 BlockColor[3]={RGBA_NONE,{},{}};//
//			
////			void SetTextColor(RGBA co)
////			{TextColor=co;}
//			
////			void SetRowColor(int p,RGBA co)
////			{RowColor[p]=co;}
//			
//			void SetBackGroundColor(RGBA co)
//			{fa2->LayerColor=co;}
//			
////			void SetEachHeight(int _x)
////			{EachHeight=_x;}
////			
////			void SetInterval(int _x)
////			{Interval=_x;}
//			
//			void SetListFunc(void (*_func)(void*,int,int))
//			{func=_func;}
//			
//			void SetDeletFunc(void (*_deletefunc)(void*))
//			{DeleteFunc=_deletefunc;}
//			
//			void SetListContent(int p,string str,void *_funcdata)//p: 0<=p<ListCnt:SetInP >=ListCnt:SetInLast <0:SetInFirst
//			{
//				p=EnsureInRange(p,0,ListCnt);
//				ListText.insert(ListText.begin()+p,str);
//				FuncData.insert(FuncData.begin()+p,_funcdata);
//				++ListCnt;
//				rPS.h=EnsureInRange(ListCnt*(EachHeight+Interval)-Interval,fa2->rPS.h,1e9);
//				fa2->ResizeLL(-1,rPS.h);
//				//CalcPS();
//				CalcPsEx();
//				NeedFreshScreenFlag=1;
//			}
//			
//			void DeleteListContent(int p)//p: 0<=p<ListCnt:SetInP >=ListCnt:SetInLast <0:SetInFirst
//			{
//				if (!ListCnt) return;
//				p=EnsureInRange(p,0,ListCnt-1);
//				if (ChooseWhich2==p) ChooseWhich2=-1;//??
//				if (DeleteFunc!=NULL) DeleteFunc(FuncData[p]);//??
//				ListText.erase(ListText.begin()+p);
//				FuncData.erase(FuncData.begin()+p);
//				--ListCnt;
//				rPS.h=EnsureInRange(ListCnt*(EachHeight+Interval)-Interval,fa2->rPS.h,1e9);
//				fa2->ResizeLL(-1,rPS.h);
//				//CalcPS();
//				CalcPsEx();
//				NeedFreshScreenFlag=1;
//			}
//			
//			void ClearListContent()
//			{
//				if (!ListCnt) return;
//				DD<<"Clear List "<<ListCnt<<"\n";
//				ChooseWhich2=-1;//??
//				if (DeleteFunc!=NULL)
//					for (vector <void*>::iterator p=FuncData.begin();p!=FuncData.end();++p)
//						DeleteFunc(*p),DD<<".";//??
//						DD<<"\n";
//				ListText.clear();
//				FuncData.clear();
//				ListCnt=0;
//				rPS.h=fa2->rPS.h;
//				fa2->ResizeLL(-1,rPS.h);
//				//fa2->ResetDeltaPos();//??
//				fa2->SetViewPort(1,0);
//				fa2->SetViewPort(2,0);
//				//CalcPS();
//				CalcPsEx();
//				NeedFreshScreenFlag=1;
//			}
//			
//			virtual void CalcPsEx()
//			{
//				if (PsEx!=NULL) PsEx->GetrPS();
//				rPS.h=EnsureInRange(ListCnt*(EachHeight+Interval)-Interval,fa2->rPS.h,1e9);
//				if (fa!=NULL)
//					gPS=rPS+fa->gPS+fa->deltaPos;
//				else gPS=rPS;
//				fa2->ResizeLL(rPS.w,rPS.h);
//			}
//			
////			virtual void Check(SDL_Event &event)
////			{
////				if (event.type==SDL_KEYDOWN)
////					if (event.key.keysym.sym==SDLK_INSERT)
////						SetListContent(1e9,"Choice "+llTOstr(ListCnt),new string("Str "+llTOstr(ListCnt)));
////					else if (event.key.keysym.sym==SDLK_DELETE)
////						DeleteListContent(1e9);
////			}
//			
//			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
//			{
//				if (ListCnt==0) return;
//				if (event.type==SDL_MOUSEBUTTONDOWN)
//				{
//					ChooseWhich1=(pt.y-gPS.y)/(EachHeight+Interval);
//					if (InRange(ChooseWhich1,0,ListCnt-1)&&
//						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight))
//					{	
//						if (event.button.button==SDL_BUTTON_LEFT)
//							stat=4,MouseEventFlag=-1;
//						else if (event.button.button==SDL_BUTTON_RIGHT)
//							stat=3,MouseEventFlag=-1;
//							
//						if (event.button.clicks==2)
//						{
//							if (event.button.button==SDL_BUTTON_LEFT)
//								if (ChooseWhich1==ChooseWhich2)
//									stat=2,ChooseWhich2=-1;
//						}
//						else if (event.button.clicks==1,1)//??
//							ChooseWhich2=ChooseWhich1;
//							
//						//DD<<"Click "<<(int)event.button.clicks<<"\n";
//						NeedFreshScreenFlag=1;
//					}
//					else stat=0,ChooseWhich1=ChooseWhich2=-1,NeedFreshScreenFlag=1;
//				}
//				else if (event.type==SDL_MOUSEBUTTONUP&&stat!=0&&stat!=1)
//				{	
//					ChooseWhich1=(pt.y-gPS.y)/(EachHeight+Interval);
//					if (InRange(ChooseWhich1,0,ListCnt-1)&&
//						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight))
//					{
//						if (func!=NULL)
//							if (stat==2) func(FuncData[ChooseWhich1],ChooseWhich1/*???*/,2);
//							else if (stat==3) func(FuncData[ChooseWhich1],ChooseWhich1,3);
//							else if (stat==4) func(FuncData[ChooseWhich2],ChooseWhich1,1);
//						stat=1;
//					}
//					else stat=0,ChooseWhich1=-1;
//					NeedFreshScreenFlag=1;
//				}
//				else if (event.type==SDL_MOUSEMOTION&&stat==0)
//				{
//					ChooseWhich1=(pt.y-gPS.y)/(EachHeight+Interval);
//					if (InRange(ChooseWhich1,0,ListCnt-1)&&
//						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight))
//						stat=1,NeedFreshScreenFlag=1;
//					else stat=0,ChooseWhich1=-1;
//				}
//			}
//			
//			virtual void LoseFocus(Point &pt,Posize &lmt)
//			{
//				lmt=lmt&gPS;
//				if (stat!=0)
//				{
//					if (!lmt.In(pt)||!(InRange(ChooseWhich1,0,ListCnt-1)&&
//						InRange(pt.y-gPS.y+1,ChooseWhich1*(EachHeight+Interval)+1,ChooseWhich1*(EachHeight+Interval)+EachHeight)))
//						stat=0,ChooseWhich1=-1,NeedFreshScreenFlag=1;
//				}
//			}
//			
//			virtual void Show(Posize &lmt)
//			{
//				if (ListCnt==0) return;
//				int ForL=(-fa2->deltaPos.y)/(EachHeight+Interval),
//					ForR=ForL+(fa2->gPS.h)/(EachHeight+Interval)+1;
//				ForL=EnsureInRange(ForL,0,ListCnt-1);
//				ForR=EnsureInRange(ForR,0,ListCnt-1);
//				if (!InRange(ChooseWhich1,0,ListCnt-1)) ChooseWhich1=-1;
//				if (!InRange(ChooseWhich2,0,ListCnt-1)) ChooseWhich2=-1;
////				DD<<"SimpleListView "<<ID<<"\n"
////				  <<"Display "<<"ForL "<<ForL<<" ForR "<<ForR<<"\n"
////				  <<"	ChooseWhich1 "<<ChooseWhich1<<"\n"
////				  <<"	ChooseWhich2 "<<ChooseWhich2<<"\n"
////				  <<"	Stat "<<stat<<"\n";
//				Posize RowPs=MakePosize(gPS.x,gPS.y+ForL*(EachHeight+Interval),gPS.w,EachHeight);
//				for (int i=ForL;i<=ForR;RowPs.y+=EachHeight+Interval,++i)
//				{
//					RenderFillRect(RowPs&lmt,RowColor[ChooseWhich2==i?2:(ChooseWhich1==i?(stat==3||stat==4?2:stat):0)]);
//					SDL_Surface *sur=CreateRGBATextSurface(ListText[i].c_str(),TextColor);
//					SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
//					Posize _pps;
//					RenderCopy(tex,(lmt-RowPs)&MakePosize(0,0,sur->w,sur->h),
//									_pps=lmt&MakePosize(RowPs.x+5,((RowPs.h-sur->h)>>1)+RowPs.y,sur->w,sur->h));
//					if (DEBUG_DisplayBorderFlag)
//						Debug_DisplayBorder(_pps);
//					SDL_FreeSurface(sur);
//					SDL_DestroyTexture(tex);
//				}	
//					
//				if (DEBUG_DisplayBorderFlag)
//					Debug_DisplayBorder(gPS);
//			}
//			
//			void AddPsEx(PosizeEX *psex)
//			{
//				if (fa2->PsEx!=NULL)
//					psex->nxt=fa2->PsEx;
//				fa2->PsEx=psex;
//				fa2->PsEx->wg=fa2;
//				fa2->CalcPsEx();
//				NeedUpdatePosize=1;
//			}
//			
//			SimpleListView(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
//			{
//				DD<<"Create SimpleListView "<<_ID<<"\n";
//				SetID(_ID);visible=_visible;
//				fa2=new LargeLayerWithScrollBar(ID*100000+1,1,_fa,_rPS,_rPS.w,_rPS.h);
//				fa2->SetLayerColor({0,0,0,20});
//				BuildRelation(fa2,this);
//				rPS={0,0,_rPS.w,0};//CalcPS();
//				Widgets::AddPsEx(new PosizeEX_BindrPS(fa2,PosizeEX_BindrPS::W_CX2));
//			}
//	};

	class SimpleBlockView:public Widgets
	{
		public:
			//bool Align=0; 
			int stat=0;//0:Up_NoFocus 1:Up_Focus_Block 2:Down_Left 3:Down_Right 4:LeftClick_Choose_NotFunc
			int BlockCnt=0;
			vector <string> MainBlockText;
			vector <string> SubBlockText;
			vector <void*> FuncData;
			vector <SDL_Texture*> BlockPic;
			void (*func)(void*,int,int)=NULL;//int1:Pos   int2: 0:None 1:Left_Click 2:Left_Double_Click 3:Right_Click
			void (*DeleteFunc)(void*,SDL_Texture*)=NULL;
			LargeLayerWithScrollBar *fa2=NULL;
			int ChooseWhich1=-1,ChooseWhich2=-1;//1:FocusChoose 2:Click_Choose
//			int EachHeight=20,
//				Interval=2;
			Posize EachPs={5,5,240,80};
			RGBA MainTextColor=RGBA_BLACK,
				 SubTextColor=RGBA_GRAY_8[4],
				 BlockColor[3]={{200,200,200,80},{0,100,255,100},{0,100,255,200}};//NoFocusRow,FocusBlock,ClickBlock
			bool EnablePic=1;
			
			void SetEnablePic(bool enable)
			{EnablePic=enable;}
			
			void SetMainTextColor(RGBA co)
			{MainTextColor=co;}
			
			void SetSubTextColor(RGBA co)
			{SubTextColor=co;}
			
			void SetBlockColor(int p,RGBA co)
			{BlockColor[p]=co;}
			
			void SetBackGroundColor(RGBA co)
			{fa2->LayerColor=co;}
			
			void SetEachPs(Posize ps)
			{EachPs=ps;}
			
			void SetBlockFunc(void (*_func)(void*,int,int))
			{func=_func;}
			
			void SetDeletFunc(void (*_deletefunc)(void*,SDL_Texture*))
			{DeleteFunc=_deletefunc;}
			
			void SetBlockContent(int p,string MainStr,string SubStr,SDL_Texture *tex,void *_funcdata)//p: 0<=p<ListCnt:SetInP >=ListCnt:SetInLast <0:SetInFirst
			{
				p=EnsureInRange(p,0,BlockCnt);
				MainBlockText.insert(MainBlockText.begin()+p,MainStr);
				SubBlockText.insert(SubBlockText.begin()+p,SubStr);
				FuncData.insert(FuncData.begin()+p,_funcdata);
				BlockPic.insert(BlockPic.begin()+p,tex);
				++BlockCnt;
				rPS.h=EnsureInRange(ceil(BlockCnt*1.0/max(1,(rPS.w+EachPs.x)/(EachPs.w+EachPs.x)))*(EachPs.y+EachPs.h)-EachPs.y,fa2->rPS.h,1e9);
				fa2->ResizeLL(-1,rPS.h);
				//CalcPS();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			void DeleteBlockContent(int p)//p: 0<=p<ListCnt:SetInP >=ListCnt:SetInLast <0:SetInFirst
			{
				if (!BlockCnt) return;
				p=EnsureInRange(p,0,BlockCnt-1);
				if (ChooseWhich2==p) ChooseWhich2=-1;//??
				if (DeleteFunc!=NULL) DeleteFunc(FuncData[p],BlockPic[p]);//??
				MainBlockText.erase(MainBlockText.begin()+p);
				SubBlockText.erase(SubBlockText.begin()+p);
				FuncData.erase(FuncData.begin()+p);
				BlockPic.erase(BlockPic.begin()+p);
				--BlockCnt;
				rPS.h=EnsureInRange(ceil(BlockCnt*1.0/max(1,(rPS.w+EachPs.x)/(EachPs.w+EachPs.x)))*(EachPs.y+EachPs.h)-EachPs.y,fa2->rPS.h,1e9);
				fa2->ResizeLL(-1,rPS.h);
				//CalcPS();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			void ClearBlockContent()
			{
				if (!BlockCnt) return;
				DD<<"Clear Block "<<BlockCnt<<"\n";
				ChooseWhich2=-1;//??
				if (DeleteFunc!=NULL)
					for (int i=0;i<BlockCnt;++i)
						DeleteFunc(FuncData[i],BlockPic[i]);
				MainBlockText.clear();
				SubBlockText.clear();
				FuncData.clear();
				BlockPic.clear();
				BlockCnt=0;
				rPS.h=fa2->rPS.h;
				fa2->ResizeLL(-1,rPS.h);
				//fa2->ResetDeltaPos();//??
				fa2->SetViewPort(1,0);
				fa2->SetViewPort(2,0);
				//CalcPS();
				CalcPsEx();
				NeedFreshScreenFlag=1;
			}
			
			virtual void CalcPsEx()
			{
				if (PsEx!=NULL) PsEx->GetrPS();
				rPS.h=EnsureInRange(ceil(BlockCnt*1.0/max(1,(rPS.w+EachPs.x)/(EachPs.w+EachPs.x)))*(EachPs.y+EachPs.h)-EachPs.y,fa2->rPS.h,1e9);
				if (fa!=NULL)
					gPS=rPS+fa->gPS+fa->deltaPos;
				else gPS=rPS;
				fa2->ResizeLL(rPS.w,rPS.h);
			}
			
//			virtual void Check(SDL_Event &event)
//			{
//				if (event.type==SDL_KEYDOWN)
//					if (event.key.keysym.sym==SDLK_INSERT)
//						SetListContent(1e9,"Choice "+llTOstr(ListCnt),new string("Str "+llTOstr(ListCnt)));
//					else if (event.key.keysym.sym==SDLK_DELETE)
//						DeleteListContent(1e9);
//			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (BlockCnt==0) return;
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					ChooseWhich1=(pt.y-gPS.y)/(EachPs.y+EachPs.h)*max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w))+(pt.x-gPS.x+EachPs.x)/(EachPs.x+EachPs.w);//???
					if (InRange(ChooseWhich1,0,BlockCnt-1)&&
						InRange(pt.y-gPS.y+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+EachPs.h)&&
						InRange(pt.x-gPS.x+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+EachPs.w))
					{	
						if (event.button.button==SDL_BUTTON_LEFT)
							stat=4,MouseEventFlag=-1;
						else if (event.button.button==SDL_BUTTON_RIGHT)
							stat=3,MouseEventFlag=-1;
							
						if (event.button.clicks==2)
						{
							if (event.button.button==SDL_BUTTON_LEFT)
								if (ChooseWhich1==ChooseWhich2)
									stat=2,ChooseWhich2=-1;
						}
						else if (event.button.clicks==1,1)//??
							ChooseWhich2=ChooseWhich1;
							
						//DD<<"Click "<<(int)event.button.clicks<<"\n";
						NeedFreshScreenFlag=1;
					}
					else stat=0,ChooseWhich1=ChooseWhich2=-1,NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&stat!=0&&stat!=1)
				{	
					ChooseWhich1=(pt.y-gPS.y)/(EachPs.y+EachPs.h)*max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w))+(pt.x-gPS.x+EachPs.x)/(EachPs.x+EachPs.w);
					if (InRange(ChooseWhich1,0,BlockCnt-1)&&
						InRange(pt.y-gPS.y+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+EachPs.h)&&
						InRange(pt.x-gPS.x+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+EachPs.w))
					{
						if (func!=NULL)
							if (stat==2) func(FuncData[ChooseWhich1],ChooseWhich1/*???*/,2);
							else if (stat==3) func(FuncData[ChooseWhich1],ChooseWhich1,3);
							else if (stat==4) func(FuncData[ChooseWhich2],ChooseWhich1,1);
						stat=1;
					}
					else stat=0,ChooseWhich1=-1;
					NeedFreshScreenFlag=1;
				}
				else if (event.type==SDL_MOUSEMOTION&&stat==0)
				{
					ChooseWhich1=(pt.y-gPS.y)/(EachPs.y+EachPs.h)*max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w))+(pt.x-gPS.x+EachPs.x)/(EachPs.x+EachPs.w);
					if (InRange(ChooseWhich1,0,BlockCnt-1)&&
						InRange(pt.y-gPS.y+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+EachPs.h)&&
						InRange(pt.x-gPS.x+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+EachPs.w))
						stat=1,NeedFreshScreenFlag=1;
					else stat=0,ChooseWhich1=-1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (stat!=0)
				{
					if (!lmt.In(pt)||!(InRange(ChooseWhich1,0,BlockCnt-1)&&
						InRange(pt.y-gPS.y+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+1,(ChooseWhich1/max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.y+EachPs.h)+EachPs.h)&&
						InRange(pt.x-gPS.x+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+1,(ChooseWhich1%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)))*(EachPs.x+EachPs.w)+EachPs.w)))
						stat=0,ChooseWhich1=-1,NeedFreshScreenFlag=1;
				}
			}
			
			virtual void Show(Posize &lmt)
			{
				if (BlockCnt==0) return;
				int ForL=((-fa2->deltaPos.y)/(EachPs.y+EachPs.h))*max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)),
					ForR=ForL+((fa2->gPS.h)/(EachPs.y+EachPs.h)+2)*max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w))-1;//??
				ForL=EnsureInRange(ForL,0,BlockCnt-1);
				ForR=EnsureInRange(ForR,0,BlockCnt-1);
				if (!InRange(ChooseWhich1,0,BlockCnt-1)) ChooseWhich1=-1;
				if (!InRange(ChooseWhich2,0,BlockCnt-1)) ChooseWhich2=-1;
//				DD<<"SimpleListView "<<ID<<"\n"
//				  <<"Display "<<"ForL "<<ForL<<" ForR "<<ForR<<"\n"
//				  <<"	ChooseWhich1 "<<ChooseWhich1<<"\n"
//				  <<"	ChooseWhich2 "<<ChooseWhich2<<"\n"
//				  <<"	Stat "<<stat<<"\n";
				Posize BlockPs=MakePosize(gPS.x+(ForR-ForL)%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w)),
											gPS.y+((-fa2->deltaPos.y)/(EachPs.y+EachPs.h))*(EachPs.y+EachPs.h),
											EachPs.w,EachPs.h);
				for (int i=ForL;i<=ForR;++i)
				{
					if (EachPs.w>=EachPs.h)
						RenderFillRect(MakePosize(BlockPs.x+BlockPs.h*EnablePic*(BlockPic[i]!=NULL),BlockPs.y,BlockPs.w-BlockPs.h*EnablePic*(BlockPic[i]!=NULL),BlockPs.h)&lmt,BlockColor[ChooseWhich2==i?2:(ChooseWhich1==i?(stat==3||stat==4?2:stat):0)]);
					else RenderFillRect(MakePosize(BlockPs.x,BlockPs.y+BlockPs.w*EnablePic*(BlockPic[i]!=NULL),BlockPs.w,BlockPs.h-BlockPs.w*EnablePic*(BlockPic[i]!=NULL))&lmt,BlockColor[ChooseWhich2==i?2:(ChooseWhich1==i?(stat==3||stat==4?2:stat):0)]);
					
					if (DEBUG_DisplayBorderFlag)
						Debug_DisplayBorder(BlockPs);
					
					Posize _pps;
					if (EnablePic&&BlockPic[i]!=NULL)
					{
						RenderCopyWithLmt(BlockPic[i],ZERO_POSIZE,_pps=MakePosize(BlockPs.x,BlockPs.y,min(BlockPs.w,BlockPs.h),min(BlockPs.w,BlockPs.h)),lmt);
						if (DEBUG_DisplayBorderFlag)
							Debug_DisplayBorder(_pps);
					}
						
					SDL_Surface *sur=CreateRGBATextSurface(MainBlockText[i].c_str(),MainTextColor);
					SDL_Texture *tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
//					RenderCopy(tex,(lmt-RowPs)&MakePosize(0,0,sur->w,sur->h),
//									_pps=lmt&MakePosize(RowPs.x+5,((RowPs.h-sur->h)>>1)+RowPs.y,sur->w,sur->h));
					if (EachPs.w<EachPs.h&&EnablePic)
						RenderCopy(tex,((lmt&BlockPs)-MakePoint(BlockPs.x,BlockPs.y+BlockPs.w))&MakePosize(0,0,sur->w,sur->h),
									_pps=lmt&BlockPs&MakePosize((BlockPs.w-sur->w>>1)+BlockPs.x,(((BlockPs.h-BlockPs.w)/2-sur->h)>>1)+BlockPs.y+BlockPs.w,sur->w,sur->h));
					else RenderCopy(tex,((lmt&BlockPs)-MakePoint(BlockPs.x+BlockPs.h*EnablePic,BlockPs.y))&MakePosize(0,0,sur->w,sur->h),
									_pps=lmt&BlockPs&MakePosize(BlockPs.x+BlockPs.h*EnablePic+5,((BlockPs.h/2-sur->h)>>1)+BlockPs.y,sur->w,sur->h));
					if (DEBUG_DisplayBorderFlag)
						Debug_DisplayBorder(_pps);
					SDL_FreeSurface(sur);
					SDL_DestroyTexture(tex);
					
					sur=CreateRGBATextSurface(SubBlockText[i].c_str(),SubTextColor);
					tex=SDL_CreateTextureFromSurface(PUI_Ren,sur);
					if (EachPs.w<EachPs.h&&EnablePic)
						RenderCopy(tex,((lmt&BlockPs)-MakePoint(BlockPs.x,BlockPs.y+BlockPs.w+(BlockPs.h-BlockPs.w)/2))&MakePosize(0,0,sur->w,sur->h),
									_pps=lmt&BlockPs&MakePosize((BlockPs.w-sur->w>>1)+BlockPs.x,(((BlockPs.h-BlockPs.w)/2-sur->h)>>1)+BlockPs.y+BlockPs.w+(BlockPs.h-BlockPs.w)/2,sur->w,sur->h));
					else RenderCopy(tex,((lmt&BlockPs)-MakePoint(BlockPs.x+BlockPs.h*EnablePic,BlockPs.y+BlockPs.h/2))&MakePosize(0,0,sur->w,sur->h),
									_pps=lmt&MakePosize(BlockPs.x+BlockPs.h*EnablePic+5,((BlockPs.h/2-sur->h)>>1)+BlockPs.y+BlockPs.h/2,sur->w,sur->h));
					if (DEBUG_DisplayBorderFlag)
						Debug_DisplayBorder(_pps);
					SDL_FreeSurface(sur);
					SDL_DestroyTexture(tex);
					
					if ((i+1)%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w))==0)
						BlockPs.y+=EachPs.y+EachPs.h,BlockPs.x=gPS.x+(ForR-ForL)%max(1,(rPS.w+EachPs.x)/(EachPs.x+EachPs.w));
					else BlockPs.x+=EachPs.x+EachPs.w;
				}	
					
				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
			
			void AddPsEx(PosizeEX *psex)
			{
				if (fa2->PsEx!=NULL)
					psex->nxt=fa2->PsEx;
				fa2->PsEx=psex;
				fa2->PsEx->wg=fa2;
				fa2->CalcPsEx();
				NeedUpdatePosize=1;
			}
			
			~SimpleBlockView()
			{
				DD<<"Delete SimpleBlockView "<<ID<<"\n";
				ClearBlockContent();
			}
			
			SimpleBlockView(int _ID,bool _visible,Widgets *_fa,Posize _rPS)
			{
				DD<<"Create SimpleBlockView "<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				fa2=new LargeLayerWithScrollBar(ID*100000+1,1,_fa,_rPS,_rPS.w,_rPS.h);
				fa2->SetLayerColor({0,0,0,20});
				BuildRelation(fa2,this);
				rPS={0,0,_rPS.w,0};//CalcPS();
				Widgets::AddPsEx(new PosizeEX_BindrPS(fa2,PosizeEX_BindrPS::W_CX2));
			}
	};
	
	class WindowDragLayer:public Widgets
	{
		public:
			int down=0;//0:Up_NoFocus 1:Up_Focus 2:Down
			int mode=0;/*0:MoveWindow 1:ResizeWindow_RD 2:ResizeWindow_RU 3:ResizeWindow_LD 4:ResizeWindow_LU
						 9:Auto		  5:ResizeWindow_U  6:ResizeWindow_D  7:ResizeWindow_L  8:ResizeWindow_R*/
			RGBA ButtonColor[3]={RGBA_NONE,RGBA_NONE,RGBA_NONE};
			Point PinPoint;
			bool IntervalTimerOn=0;
			SDL_TimerID IntervalTimerID;
			Uint32 UpdateInterval=10;
			
			void SetColor(int p,RGBA co)
			{ButtonColor[p]=co;}
			
			void TurnOnOffIntervalTimer(bool on)
			{
				if (IntervalTimerOn==on) return;
				PUI_UpdateTimerData *ptr=NULL;
				if (on)
				{
					PUI_UpdateTimerData *p=new PUI_UpdateTimerData;
					ptr=p;
					p->cnt=-1;
					p->tar=this;
					IntervalTimerID=SDL_AddTimer(UpdateInterval,PUI_UpdateTimer,p);
					IntervalTimerOn=1;
				}
				else//Do remember use it when deconstruct this;
				{
//					*(ptr->enableFlag)=0;//not fixed yet//attention (memory leak):same bug in TextEditLine needFix;
					SDL_RemoveTimer(IntervalTimerID);
					IntervalTimerOn=0;
				}
			}
			
			void Func()
			{
				int md=mode;
				if (mode==9)
				{
					if (1)
						md=1;
				}
				 
				Point NowGlobalPos;
				SDL_GetGlobalMouseState(&NowGlobalPos.x,&NowGlobalPos.y);
				Posize winPS={WindowPos.x,WindowPos.y,PUI_WindowWidth,PUI_WindowHeight};
				
				if (!(NowGlobalPos-WindowPos==PinPoint))
					switch (md)
					{
						case 0: SetWindowPosition(NowGlobalPos-PinPoint); 	break;
						case 1:	SetWindowSize(NowGlobalPos-WindowPos-PinPoint+MakePoint(PUI_WindowWidth,PUI_WindowHeight));
								PinPoint=NowGlobalPos-WindowPos;NeedFreshScreenFlag=1;			break;

						default:
							DD<<"Invalid Mode value\n";
					}
			}
			
			virtual void Check(SDL_Event &event)
			{
				if (event.type==SDL_USEREVENT)
					if (event.user.type==PUI_EVENT_PRESENT)
						if (event.user.data1==this)
							if (down==2) Func();
							else TurnOnOffIntervalTimer(0);
			}
			
			virtual void CheckPos(SDL_Event &event,Point &pt,int &MouseEventFlag)
			{
				if (event.type==SDL_MOUSEBUTTONDOWN)
				{
					DD<<"WindowDragLayer Down "<<ID<<"\n";
					if (event.button.button==SDL_BUTTON_LEFT)
						if (gPS.In(pt))
						{
							OccupyPosState=1;
							OccupyPosWd=this;
							down=2;
							PinPoint=pt;
							TurnOnOffIntervalTimer(1);
							MouseEventFlag=-1;
							NeedFreshScreenFlag=1;
						}
				}
				else if (event.type==SDL_MOUSEBUTTONUP&&down==2)
				{
					DD<<"WindowDragLayer Up "<<ID<<"\n";
//					if (gPS.In(pt))
					{
						OccupyPosState=0;
						OccupyPosWd=NULL;
						down=1;
						TurnOnOffIntervalTimer(0);
						NeedFreshScreenFlag=1;
					}
				}
				else if (event.type==SDL_MOUSEMOTION&&down==0)
				{
					DD<<"WindowDragLayer Focus "<<ID<<"\n";
					if (gPS.In(pt))
						down=1;
					NeedFreshScreenFlag=1;
				}
			}
			
			virtual void LoseFocus(Point &pt,Posize &lmt)
			{
				lmt=lmt&gPS;
				if (down!=0)
					if (!lmt.In(pt))
						down=0,NeedFreshScreenFlag=1,DD<<"WindowDragLayer LoseFocus "<<ID<<"\n";
			}
			
			virtual void Show(Posize &lmt)
			{
				RenderFillRect(lmt=gPS&lmt,ButtonColor[down]);

				if (DEBUG_DisplayBorderFlag)
					Debug_DisplayBorder(gPS);
			}
					
			WindowDragLayer(int _ID,bool _visible,Widgets *_fa,Posize _rPS,int _mode)
			{
				DD<<"Create WindowDragLayer"<<_ID<<"\n";
				SetID(_ID);visible=_visible;
				BuildRelation(_fa,this);
				rPS=_rPS;//CalcPS();
				CalcPsEx();
				mode=_mode;
			}
	};
	
	class SimpleTreeView:public Widgets
	{
		public:
			
	};
	
	class Menu2
	{
		public:
			
	};
	
	Layer *BackGroundLayer=NULL;
	
	void UpdateWindowSize(int WinWidth,int WinHeight)
	{
		PUI_WindowWidth=WinWidth;
		PUI_WindowHeight=WinHeight;
		BackGroundLayer->Resize(WinWidth,WinHeight);
//		BackGroundLayer->CalcPS();
		BackGroundLayer->gPS=BackGroundLayer->rPS;
		MenusLayer->Resize(WinWidth,WinHeight);
		MenusLayer->gPS=MenusLayer->rPS;
	}
	
	void _UpdateAllPs(Widgets *wg)
	{
		if (wg==NULL) return;
		_UpdateAllPs(wg->brother);
		if (!wg->visible) return;
		wg->CalcPsEx();
		_UpdateAllPs(wg->childWg);
	}
	
	void UpdateAllPs()
	{
	//	BackGroundLayer->rPS={0,0,PUI_WindowWidth,PUI_WindowHeight};
		_UpdateAllPs(BackGroundLayer->childWg);
		_UpdateAllPs(MenusLayer->childWg);
	}
	
	void PresentWidgets()
	{
		SDL_SetRenderDrawColor(PUI_Ren,255,255,255,255);
		SDL_RenderClear(PUI_Ren);
		BackGroundLayer->_PresentWidgets(BackGroundLayer->gPS);
		MenusLayer->_PresentWidgets(MenusLayer->gPS);
	}
	
	void PresentWidgets(Posize lmt)
	{
//		SDL_SetRenderDrawColor(PUI_Ren,255,255,255,255);
//		SDL_RenderClear(PUI_Ren);
		BackGroundLayer->_PresentWidgets(lmt);
		MenusLayer->_PresentWidgets(lmt);
	}
	
	void PresentWidgets(Widgets *tar)
	{
		tar->_PresentWidgets(tar->gPS);
		MenusLayer->_PresentWidgets(tar->gPS);
	}
	
	void SolveEvent(SDL_Event &event)
	{
		PUI_NowSolvingEvent=&event;
		if (event.type==SDL_WINDOWEVENT)
		{
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_RESIZED:
					DD<<"Window Resize to "<<event.window.data1<<" "<<event.window.data2<<"\n";
					UpdateWindowSize(event.window.data1,event.window.data2);
					NeedFreshScreenFlag=1;
					break;
				default:
					EventSolvedFlag=0;
					MenusLayer->_SolveEvent(event);
					BackGroundLayer->_SolveEvent(event);
			}
		}
		
//		if (TopWidget!=NULL)
//		{
//			
//		}

//		else if (event.type==SDL_USEREVENT)
//		{
//			if (event.user.type==PUI_EVENT_PRESENT)
//			{
//				PresentWidgets(((Widgets*)event.user.data1)->gPS);
//			}
//		}
		
		else if (event.type==SDL_TEXTINPUT||event.type==SDL_TEXTEDITING)
		{
			if (KeyboardInputWgState==1&&KeyboardInputWg!=0)
			{
				DD<<"ReceiveKeyboardInput "<<CharSet_Net::Utf8ToAnsi(event.text.text)<<"\n";
				KeyboardInputWg->ReceiveKeyboardInput(event);
			}
		}
		
		
		else if (event.type==SDL_MOUSEBUTTONUP||event.type==SDL_MOUSEBUTTONDOWN
			||event.type==SDL_MOUSEMOTION)
		{
			Point pt;
			int MouseEventFlag=1;
			pt.GetSDLEventPos(event);
			NowPos=pt;
			if (OccupyPosState==1&&OccupyPosWd!=NULL)
			{
				if (OccupyPosWd->visible)
					OccupyPosWd->CheckPos(event,pt,MouseEventFlag);
				else DD<<"OccupyPosWd "<<OccupyPosWd->ID<<" Unvisible!"<<"\n";
			}
			else if (OccupyPosState==0)
			{
				MenusLayer->_SolveLoseFocusEvent(pt,{0,0,PUI_WindowWidth,PUI_WindowHeight}),
				MenusLayer->_SolvePosEvent(event,pt,MouseEventFlag);
				BackGroundLayer->_SolveLoseFocusEvent(pt,{0,0,PUI_WindowWidth,PUI_WindowHeight}),
				BackGroundLayer->_SolvePosEvent(event,pt,MouseEventFlag);
			}
				
		}
		else 
		{
			EventSolvedFlag=0;
			MenusLayer->_SolveEvent(event);
			BackGroundLayer->_SolveEvent(event);
		}
		
		PUI_NowSolvingEvent=NULL;
	}
	
	void PAL_GUI_Alpha_Init(int WinWidth,int WinHeight)
	{
		DD<<"PAL_GUI_Alpha_Init\n";
		SDL_Init(SDL_INIT_EVERYTHING);
		TTF_Init();
		
		PUI_Cfg.Read();
		
		
		PUI_Win=SDL_CreateWindow(WindowTitle.c_str(),WindowPos.x,WindowPos.y,PUI_WindowWidth=WinWidth,PUI_WindowHeight=WinHeight,SDL_WINDOW_RESIZABLE);
		
		if (PUI_Cfg.GetValue("Disable_ACCELERATED")=="1")
			PUI_Ren=SDL_CreateRenderer(PUI_Win,-1,SDL_RENDERER_SOFTWARE),
			DD<<"Disable_ACCELERATED\n";
		else
			PUI_Ren=SDL_CreateRenderer(PUI_Win,-1,SDL_RENDERER_ACCELERATED
											//	|SDL_RENDERER_PRESENTVSYNC
												);
	
//		SDL_CreateWindowAndRenderer(PUI_WindowWidth=WinWidth,PUI_WindowHeight=WinHeight,SDL_WINDOW_RESIZABLE,&PUI_Win,&PUI_Ren);	
		
		PUI_Font=TTF_OpenFont("msyhl.ttc",14);
		SDL_SetRenderDrawBlendMode(PUI_Ren,SDL_BLENDMODE_BLEND);
		
		PUI_EVENT_PRESENT=SDL_RegisterEvents(1);
//		PUI_VirtualPosEvent=SDL_RegisterEvents(1);
		
		
		BackGroundLayer=new Layer(0,1,NULL,{0,0,WinWidth,WinHeight});
		MenusLayer=new Layer(-2,1,NULL,{0,0,WinWidth,WinHeight});
	}
	
	void PAL_GUI_Alpha_Init(int WinPosX,int WinPosY,int WinWidth,int WinHeight)
	{
		PAL_GUI_Alpha_Init(WindowPos.x=WinPosX,WindowPos.y=WinPosY,WinWidth,WinHeight);
	}
	
	void PAL_GUI_Alpha_Quit()
	{
		
		
		PUI_Cfg.Write();
	}
}
//End of UI
#endif
