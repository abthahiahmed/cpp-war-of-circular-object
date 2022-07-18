#include <iostream>
#include <vector>
#include <math.h>
#include <SDL2/SDL.h>

using namespace std;

#define TWO_PI 2 * 3.1416
#define PI 3.1416

int random(int a, int b){
	return a + rand() / (RAND_MAX / (b-a));
}

void print(string str){
    cout<<str;
}
void println(string str){
    cout<<str<<endl;
}
void drawCircle(SDL_Renderer *ren, int x, int y, int r){
	float px = r * cos(0.04);
	float py = r * sin(0.04);

    for(float i = 0; i < TWO_PI; i += 0.04){
		float cx = r * cos(i);
		float cy = r * sin(i);		
        if(px != 0 && py != 0){
			SDL_RenderDrawLine(ren, x + cx, y + cy, x + px, y + py);
		}
		px = cx;
		py = cy;
    }	
}

int sign(float num){
	
	if(num < 0){
		return -1;
	}else{
		return 1;
	}
	
}
// Class for Bullets
class Bullet{	
private:
	float vx,vy; // Velocity
	float tx, ty; // Target
	float dis; // Distance Between Target and Bullet
public:
	float x,y; // Bullet Position
	Bullet(float x, float y, float tx, float ty){
		// Initialization
		this->x = x; 
		this->y = y;
		this->tx = tx;
		this->ty = ty;
		this->dis = sqrt(pow(this->tx - this->x, 2) + pow(this->ty - this->y, 2));
		this->hu = 255;
		
	}
	
	void update(){
		this->x += this->vx;		
		this->y += this->vy;
		
		this->vx = ((this->tx - this->x) / this->dis) * 10;			
		this->vy = ((this->ty - this->y) / this->dis) * 10;
		
	}
	
	bool bulletDone(){
		// Bullet Collide or not 		
		if((this->x > this->tx-2) && (this->x < this->tx + 2) 
			&& (this->y > this->ty-2) && (this->y < this->ty + 2)) {
			return true;
		}
		return false;
	}
	
	
	
	void draw(SDL_Renderer *ren){
		//Graphics
		SDL_SetRenderDrawColor(ren, 250, 250, 100, 255);
		SDL_RenderDrawPoint(ren, this->x, this->y);		
		SDL_RenderDrawPoint(ren, this->x-1, this->y-1);
		
	}
};

class Object {
	private:
		float vx, vy; // Velocity
		float dx, dy; // Random Destination
		float cx, cy; // Diretion
		double angle; // Angle of  Direction
		float range; // Range of Object
		bool tInRange; // Check if target in range
		float rx,ry; // Range Rudder
		float rAngle; // Rudder Angle
		vector<Bullet> bullets; // Bullets Vector
		bool isBusy; // Check if Object busy in firing
		int busyWith; // Id of object which is hitting by this objects bullets
	public:
		float x, y; // Object Position
		float radius; // Object Radius
		float life; // Object Life
		float id; // Object ID
	
	Object(float x, float y, int id){
		
		this->x = x;
		this->y = y;
		
		this->vx = 0;
		this->vy = 0;
		this->dx = x;
		this->dy = y;
		
		this->angle = 0;
		this->radius = 20;
		this->range = 150;
		this->tInRange = false;
		this->rAngle = 0;
		this->life = 100;
		
		this->isBusy = false;
		this->busyWith = 0;
		
		
		this->id = id;
		
		
	}
	void dLife(float l){
		this->life -= l;
	}
	
	void update(){
		
		if(!this->tInRange){
			this->angle = atan2((this->dy - this->y), (this->dx - this->x));
		}
		this->tInRange = false;
		if(this->x > this->dx+10 || this->x < this->dx-10 || this->y > this->dy+10 || this->y < this->dy-10) // Check if object on destination...
		{	
			
			float dis = sqrt(pow(this->dx - this->x, 2) + pow(this->dy - this->y, 2));
			
			this->vx = (this->dx - this->x) / dis;			
			this->vy = (this->dy - this->y) / dis;
				
			this->x += this->vx;
			this->y += this->vy;
			
		}
		this->cx = this->radius * cos(this->angle);
		this->cy = this->radius * sin(this->angle);
		
		this->rx = this->range * cos(this->rAngle);
		this->ry = this->range * sin(this->rAngle);
		
		if(rAngle > TWO_PI){ // check if rudder rotation is more than 2 times of PI.
			rAngle = 0;
		}
		rAngle += 0.05;
		
		for(unsigned int n = 0; n < this->bullets.size(); n++){
			this->bullets[n].update();
			if(this->bullets[n].bulletDone()){
				this->bullets.erase(this->bullets.begin() + n); // Delete used bullets
			}
			
			
		}
		
		
		
	}
	
	bool impact(Object obj){
		for(unsigned int n=0;n<this->bullets.size();n++){
			
							
			float _x1 = this->bullets[n].x;
			float _y1 = this->bullets[n].y;
			
			float _x2 = obj.x;
			float _y2 = obj.y;
			float _r2 = obj.radius;
			
			
			
			
			if((_x1 > _x2 - _r2/2 ) && (_x1 < _x2 + _r2/2) &&
				(_y1 > _y2 - _r2/2 ) && (_y1 < _y2 + _r2/2)){
				return true;
			}
		}
		return false;
	}
	
	bool targetInRange(Object obj){
		
		if(!this->isBusy){
			this->busyWith = obj.id;
		}
		
		float x1 = this->x;
		float y1 = this->y;
		float r1 = this->range + obj.radius;
		float x2 = obj.x;
		float y2 = obj.y;
		float r2 = obj.radius;
		float Dx = pow(x2 - x1, 2);
		float Dy = pow(y2 - y1, 2);
		float R = pow(r2 - r1, 2);
		float d = Dx + Dy;
		if(d <= R){
			this->tInRange = true;
			
			if(obj.id == this->busyWith){
				this->angle = atan2((y2 - y1), (x2 - x1));
				this->bullets.push_back(Bullet(this->x, this->y, x2, y2));				
			}
			this->isBusy = true;
			return true;
		}else{
			this->isBusy = false;
			this->busyWith = 0;
			return false;
			
		}

	}
	
	
	bool checkIfReach(){
		
		if(this->x > this->dx+10 || this->x < this->dx-10 || this->y > this->dy+10 || this->y < this->dy-10)
		{	
			return false;
		}else{
			return true;
		}
		
	}
	
	void setDest(float dx, float dy){
		this->dx = dx;
		this->dy = dy;
	}
	
	void draw(SDL_Renderer *ren){
		
		//Graphics
		
		for(unsigned int n = 0; n < this->bullets.size(); n++){
			
			this->bullets[n].draw(ren);
		}
				
		
		
		SDL_SetRenderDrawColor(ren,100,255, 100, 200);
		drawCircle(ren, this->x, this->y, this->range);
		SDL_RenderDrawLine(ren, this->x, this->y, this->x+rx, this->y+ry);
		
		
		
		
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		drawCircle(ren, this->x, this->y, this->radius);
		SDL_RenderDrawLine(ren, this->x, this->y, this->x+cx, this->y+cy);
		
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 100);
		SDL_Rect prog = {
			(int)this->x - 50,
			(int)this->y - ((int)this->radius + 40),
			100,
			5
		};
		
		SDL_RenderFillRect(ren, &prog);
		SDL_SetRenderDrawColor(ren, 255, 100, 100, 200);
		SDL_Rect bar = {
			(int)this->x - 50,
			(int)this->y - ((int)this->radius + 40),
			(int)this->life,
			5
		};
		
		SDL_RenderFillRect(ren, &bar);
	}
};






int main(int argc, char **argv){
	// SDL Window Creation
    SDL_Window *win = SDL_CreateWindow("War Between Randomly Moving Circular Objects| Abthahi Ahmed Rifat", 100,100, 1000, 600, SDL_WINDOW_SHOWN);
    if(!win){
        cout<<"Error in creating Window"<<endl;
        exit(1);
    }
	//SDL Renderer Creation
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);
    if(!ren){
        cout<<"Error in creating Renderer"<<endl;
        exit(1);
    }
	// Event Create
    SDL_Event event;
	
	int width, height;
	SDL_GetRendererOutputSize(ren, &width, &height);
	
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);


	bool running = true;
	
	vector<Object> obj;
	
	// Adding Objects on Ground.
	obj.push_back(Object(random(50, width-50), random(50, height-50), 1));	
	obj.push_back(Object(random(50, width-50), random(50, height-50), 2));
	obj.push_back(Object(random(50, width-50), random(50, height-50), 3));	
	obj.push_back(Object(random(50, width-50), random(50, height-50), 4));


	
	
	while(running){
		// Background color
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);
		
		for(unsigned int i = 0; i < obj.size(); i++){
		
			obj[i].update();
			obj[i].draw(ren);
			
			
			if(obj[i].checkIfReach()){
				obj[i].setDest(random(50, width-50), random(50, height-50));
			}
			for(unsigned int j = 0; j < obj.size(); j++){
				
				if(j != i){
					obj[i].targetInRange(obj[j]);
					if(obj[i].impact(obj[j])){
						obj[j].dLife(0.1);
					}
				}
			}
			if(obj[i].life < 0) obj.erase(obj.begin()+i);
		}
	

		
		
		
	// Event listener
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT){
            running = false;
        }
	// Render Drawing
        SDL_RenderPresent(ren);
		SDL_Delay(1000/70);
		random(0,1000);
	}
    if(running == false){
        print("Program Terminated.");
    }
    return 0;
}
