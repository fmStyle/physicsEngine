#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <iostream>
#include <math.h>
#include <memory>
#define PI 3.14159265
using namespace sf;
using namespace std;



void setupBackgroundSquares(vector<RectangleShape>&squares);
float distance(Vector2f position1, Vector2f position2){
	return sqrt(pow(position1.y - position2.y, 2) + pow(position1.x - position2.x, 2));
}
float angle(Vector2f position1, Vector2f position2){
	return atan2(position2.y - position1.y, position2.x - position1.x)*180/PI - 90;
}
	
Vector2f MousePos(View &actualView, RenderWindow &w);
	
class Particle{
public:
	bool immobile;
	bool affectedByBounds;
	float m_mass;
	float m_forceX;
	float m_forceY;
	float m_radius;
	float plasticCoefficient;
	
	CircleShape shape;
	Vector2f position;
	Vector2f previousPosition;
	Particle(float mass, float forceX, float forceY, float radius, Vector2f initialPos){
		immobile = false;
		affectedByBounds = true; /// Default settings
		m_radius = radius;
		m_forceX = forceX;
		m_forceY = forceY;
		m_mass = mass;
		
		/// I may use this coefficient in the future
		plasticCoefficient = 0.8f;
		
		
		shape.setRadius(radius);
		shape.setPosition(initialPos);
		shape.setOrigin(Vector2f(radius, radius));
		shape.setOutlineColor(Color::Black);
		shape.setOutlineThickness(5.0f);	
		position = Vector2f(shape.getPosition().x, shape.getPosition().y);
		previousPosition = shape.getPosition();
		previousPosition.y += 0.1f;
	}
	void update(float &dt, RenderWindow &w){
		this->onClick(w);
		if (immobile) return;
		
		/// Verlet integration;
		float accelerationX = m_forceX/m_mass;
		float accelerationY = m_forceY/m_mass;
		float newPosX = shape.getPosition().x + (shape.getPosition().x - previousPosition.x) + accelerationX*pow(dt, 2);
		float newPosY = shape.getPosition().y + (shape.getPosition().y - previousPosition.y) + accelerationY*pow(dt, 2);
		
		previousPosition = shape.getPosition();
		Vector2f newPos = Vector2f(newPosX, newPosY);
		shape.setPosition(newPos);
		if (affectedByBounds) this->constraint();
	}
	void draw(RenderWindow &w){
		w.draw(shape);
	}
	void constraint(){
		/// Constraint against borders (This could be much better)
		if (shape.getPosition().y > 720){
			previousPosition.y = shape.getPosition().y + (shape.getPosition().y - previousPosition.y);
		}
		/// Uncomment if you want constraints in the other walls, the res is hardcoded
//		if (shape.getPosition().y < 0){
//			previousPosition.y = shape.getPosition().y + (shape.getPosition().y - previousPosition.y);
//		}
//		if (shape.getPosition().x > 1280){
//			previousPosition.x = shape.getPosition().x + (shape.getPosition().x - previousPosition.x);
//		}
//		if (shape.getPosition().x < 0){
//			previousPosition.x = shape.getPosition().x + (shape.getPosition().x - previousPosition.x);
//		}
		/// 
	}
	Vector2f getPosition(){
		return shape.getPosition();
	}
	void setPosition(Vector2f newPos){
		shape.setPosition(newPos);
	}
	void setPreviousPosition(Vector2f previousPos){
		previousPosition = previousPos;
	}
	void onClick(RenderWindow &w){
		if (distance(Vector2f(Mouse().getPosition(w)), shape.getPosition()) < this->m_radius){
			if (Mouse().isButtonPressed(Mouse::Button::Left)) shape.setPosition(Vector2f(Mouse().getPosition(w)));
			
		}
	}
	
	void collision(){
		previousPosition.x = shape.getPosition().x + (shape.getPosition().x - previousPosition.x);
	}
	~Particle(){}
};

class Stick{
public:
	shared_ptr<Particle> m_p1;
	shared_ptr<Particle> m_p2;
	float m_length;
	float m_linethickness;
	float angleBetweenPoints;
	float distanceBetweenPoints;
	float springCoefficient;
	RectangleShape line;
	Stick(shared_ptr<Particle> p1, shared_ptr<Particle> p2, float linethickness = 5.0f){
		m_p1 = p1;
		m_p2 = p2;
		m_length = distance(p1->getPosition(), p2->getPosition());
		m_linethickness = linethickness;
		line.setSize(Vector2f(linethickness, m_length));
		
		springCoefficient = 0.5f; /// Default spring coefficient, with this, the sticks remain the same length
		
		line.setOrigin(Vector2f(linethickness/2, 0));
		line.setPosition(m_p1->getPosition());
		setColors();
	}
	void update(float dt){
		angleBetweenPoints = atan2(m_p2->getPosition().y - m_p1->getPosition().y, m_p2->getPosition().x - m_p1->getPosition().x)*180/PI - 90;
		distanceBetweenPoints = distance(m_p1->getPosition(), m_p2->getPosition());
		line.setPosition(m_p1->getPosition());
		line.setSize(Vector2f(m_linethickness, distanceBetweenPoints));
		line.setRotation(angleBetweenPoints);
		
		/// Constraint Logic
		float dx = m_p2->getPosition().x - m_p1->getPosition().x;
		float dy = m_p2->getPosition().y - m_p1->getPosition().y;
		float dist = sqrt(dx*dx + dy*dy);
		float difference = m_length - dist;
		float percent = (difference/dist)*springCoefficient;
		float offsetX = dx*percent;
		float offsetY = dy*percent;
		if (!m_p1->immobile) m_p1->setPosition(Vector2f(m_p1->getPosition().x - offsetX, m_p1->getPosition().y - offsetY));
		if (!m_p2->immobile)m_p2->setPosition(Vector2f(m_p2->getPosition().x + offsetX, m_p2->getPosition().y + offsetY));
	}
	void draw (RenderWindow &w){
		
		w.draw(line);
	}
	void setColors(Color color=Color::Green, float outlinethickness=3.0f){
		line.setFillColor(color);
		line.setOutlineColor(Color::Black);
		line.setOutlineThickness(outlinethickness);
	}
	bool contains(shared_ptr<Particle> p){
		if (p == m_p1) return true;
		if (p == m_p2) return true;
		return false;
	}
	
};

/// Some button classes, not very interesting, I realized midway through that I could've just passed a pointer function
class Button{
public:
	RectangleShape button;
	Text m_text;
	float m_sizeX, m_sizeY;
	Vector2f m_position;
	float m_characterSize;
	bool m_state;
	bool m_state1;
	bool clicked;
	string tag;
	
	Button(Font &font, string text, float sizeX=100.0f, float sizeY=30.0f, Vector2f position=Vector2f(0.0f, 0.0f), float characterSize=24.0f, bool state=false){
		this->initialize(font, text, sizeX, sizeY, position, characterSize, state);
		
	}
	void initialize(Font &font, string text, float sizeX=100.0f, float sizeY=30.0f, Vector2f position=Vector2f(0.0f, 0.0f), float characterSize=24.0f, bool state=false){
		clicked = false;
		m_text.setFont(font);
		m_text.setString(text);
		button.setSize(Vector2f(sizeX, sizeY));
		button.setOrigin(Vector2f(sizeX/2.0f, sizeY/2.0f));
		button.setFillColor(Color::Black);
		button.setOutlineColor(Color::White);
		button.setOutlineThickness(5.0f);
		m_position = position;
		button.setPosition(m_position);
		m_characterSize = characterSize;
		m_text.setCharacterSize(m_characterSize);
		m_state = state;
		m_text.setPosition(button.getPosition());
		m_text.setFillColor(Color::White);
		m_text.setOrigin(button.getOrigin());
	}
	
	void update(RenderWindow &w){
		if (button.getGlobalBounds().contains(Vector2f(Mouse().getPosition(w)))){
			if (Mouse().isButtonPressed(Mouse::Button::Left) && !clicked){
				this->onClick();
				clicked = true;
			}
			if (!Mouse().isButtonPressed(Mouse::Button::Left) && clicked){
				clicked = false;
			}
		}
		this->extraUpdate(w);
	}
	void setPosition(Vector2f newPos){
		button.setPosition(newPos);
		m_text.setPosition(newPos);
	}
	virtual void extraUpdate(RenderWindow &w){}
	
	virtual void draw(RenderWindow &w){
		w.draw(button);
		w.draw(m_text);
		
	}
	virtual void onClick(){}
};
class ToggleButton : public Button{
public:
	Text toggleInfo;
	ToggleButton(Font &font, string text="info", float sizeX=26.0f, float sizeY=26.0f, Vector2f position=Vector2f(0.0f, 0.0f), float characterSize=26.0f, bool state=false) : Button(font, text, sizeX, sizeY, position, characterSize, state){
		toggleInfo.setFont(font);
		toggleInfo.setString(text);
		toggleInfo.setCharacterSize(characterSize);
		toggleInfo.setOrigin(Vector2f(0.0f, characterSize/2.0f));
		toggleInfo.setPosition(Vector2f(this->button.getPosition().x + 25.0f, this->button.getPosition().y));
		m_text.setString("");
	}
	void onClick(){
		if (!m_state){
			m_state = true;
			m_text.setString("x");
		} else {
			m_state = false;
			m_text.setString("");
		}
	}
	void draw(RenderWindow &w){
		w.draw(this->button);
		w.draw(this->m_text);
		w.draw(this->toggleInfo);
	}
};
class PlayButton : public Button{
public:
	PlayButton(Font &font, string text, float sizeX=100.0f, float sizeY=100.0f, Vector2f position=Vector2f(0.0f, 0.0f), float characterSize=24.0f, bool state=false) : Button(font, text, sizeX, sizeY, position, characterSize, state){
		tag = "play button";
	}
	
	void onClick(){
		if (!m_state){
			m_state = true;
			m_text.setString("pause");
		} else {
			m_state = false;
			m_text.setString("play");
		}
	}
	void draw(RenderWindow &w){
		w.draw(button);
		w.draw(m_text);
	}
};
class CreateParticleButton : public Button{
	
public:
	CreateParticleButton(Font &font, string text, float sizeX=200.0f, float sizeY=30.0f, Vector2f position=Vector2f(0.0f, 0.0f), float characterSize=24.0f, bool state=false) : Button(font, text, sizeX, sizeY, position, characterSize, state){
		tag = "create particle button";
	}
	
	void onClick(){
		if (!m_state){
			m_state = true;
			m_text.setString("stop");
		} else {
			m_state = false;
			m_text.setString("Create Particle");
		}
	}
};
class CreateStickButton : public Button{
public:
	CreateStickButton(Font &font, string text, float sizeX=200.0f, float sizeY=30.0f, Vector2f position=Vector2f(0.0f, 0.0f), float characterSize=24.0f, bool state=false) : Button(font, text, sizeX, sizeY, position, characterSize, state){
		tag = "create stick button";
	}
	
	void onClick(){
		if (!m_state){
			m_state = true;
			m_text.setString("stop");
		} else {
			m_state = false;
			m_text.setString("Create Stick");
		}
	}
};



int main(int argc, char *argv[]){
	/// Initializations
	Font font;
	if (!font.loadFromFile("dogica.ttf")) cout<<"Font not found"<<endl;
	
	Image icon;
	if (!icon.loadFromFile("tukodev.png")) cout<<"Icon not found"<<endl;
	
	RenderWindow w(VideoMode(1280, 720),"Verlet");
	w.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	
	vector<shared_ptr<Particle>> particles;
	vector<Stick>sticks;
	
	/// Initial Pendulum (Comment for not initialize with it)
//	shared_ptr<Particle>p1 = make_shared<Particle>(1.0f, 0.0f, 1000.0f, 15.0f, Vector2f(1280/2, 720/2)); p1->immobile = true; p1->shape.setFillColor(Color::Yellow);
//	shared_ptr<Particle>p2 = make_shared<Particle>(1.0f, 0.0f, 1000.0f, 15.0f, Vector2f(1280/2 + 80.0f, 720/2 - 200.0f));
//
//	particles.push_back(p1);
//	particles.push_back(p2); 
//
//	Stick s2(particles[0], particles[1]);
//	sticks.push_back(s2); 
	///
	
	/// Cloth Sim (Uncomment if you want to initialize with it)
	int partitionsX = 50;
	int partitionsY = 35;
	float marginX = 100.0f;
	float marginY = 0.0f;
	float deltaX = (1280.0f - marginX)/partitionsX;
	float deltaY = (1280.0f - marginY)/partitionsY;
	int counter = 0;
	for (int i = 0; i<partitionsX; ++i){
		for (int j = 0; j<partitionsY; ++j){
			
			shared_ptr<Particle>pk = make_shared<Particle>(1.0f, 300.0f, 1000.0f, 0.0f, Vector2f(marginX + deltaX*i, marginY + deltaY*j));
			pk->affectedByBounds = false;
			if ((i==0 && j== 0) || (i==partitionsX-1 && j==0)) pk->immobile = true;
			if (j==0 && i==int(partitionsX/2)) pk->immobile = true;
			particles.push_back(pk);
			if (j!=0){
				Stick sk(particles[counter], particles[counter-1], 3.0f);
				sk.line.setOutlineThickness(0.0f);
				sk.line.setFillColor(Color::White);
				sticks.push_back(sk);
			}
			if (i!= 0){
				Stick sk(particles[counter-partitionsY], particles[counter], 3.0f);
				sk.line.setOutlineThickness(0.0f);
				sk.line.setFillColor(Color::White);
				sticks.push_back(sk);
			}
			++counter;
		}
	}
	/// 
	
	vector<RectangleShape>squares;
	setupBackgroundSquares(squares);

	vector<Button*>buttons;
	
	Button *play = new PlayButton(font, "Play", 120.0f, 30.0f, Vector2f(80.0f, 50.0f), 24.0f, false);
	Button *createParticle = new CreateParticleButton(font, "create particle", 380.0f, 30.0f, Vector2f(400.0f, 50.0f), 24.0f, false);
	Button *toggleImmobile = new ToggleButton(font, "immobile", 25.0f, 25.0f, Vector2f(250.0f, 100.0f), 24.0f, false);
	Button *createStick = new CreateStickButton(font, "create stick", 320.0f, 30.0f, Vector2f(800.0f, 50.0f), 24.0f, false);
	Button *toggleSpring = new ToggleButton(font, "spring", 25.0f, 25.0f, Vector2f(700.0f, 100.0f), 24.0f, false);
	Button *clear = new Button(font, "clear", 200.0f, 30.0f, Vector2f(1100.0f, 50.0f), 24.0f, false);
	
	
	buttons.push_back(play);
	buttons.push_back(createParticle);
	buttons.push_back(createStick);
	buttons.push_back(toggleImmobile);
	buttons.push_back(toggleSpring);
	buttons.push_back(clear);
	
	Clock clock;
	
	float timer1 = .15f;
	
	bool state1 = false;
	
	shared_ptr<Particle>selectedParticle1;
	shared_ptr<Particle>selectedParticle2;
	
	View actualView;
	actualView = w.getView();
	w.setView(actualView);
	
	View guiView;
	guiView = w.getView();
	
	
	RectangleShape tempStick;
	tempStick.setFillColor(Color(0, 255, 0, 100));
	float actualZoom = 1.0f;
	/// Main window loop
	while(w.isOpen()) {
		Event e;
		while(w.pollEvent(e)) {
			if(e.type == Event::Closed)
				w.close();	
			if (e.type == sf::Event::Resized)
			{
				// update the view to the new size of the window
				FloatRect visibleArea(0.f, 0.f, e.size.width, e.size.height);
				actualView = View(visibleArea);
			}
			if(e.type == Event::MouseWheelScrolled){

				float movement = e.mouseWheelScroll.delta; // 1 for positive, -1 for negative
				actualView = w.getView();
				
				if (movement>0 && !actualZoom<0.2f) actualZoom -= 0.1f;
				if (movement<0) actualZoom += 0.1f;
				actualView.zoom(actualZoom);
				
			}
		}
		
		float dt = clock.restart().asSeconds();
		
		/// Updates for every particle, stick and button
		for (Stick &sk : sticks){
			
			sk.update(dt);
		}
		
		for (auto &bk : buttons){
			
			bk->update(w);
		}
		
		for (auto pk : particles){
			if (play->m_state == true) pk->update(dt, w);
			
		}
		
		/// Particle creation logic (The timer is because the continuous click was bugging the behaviour)
		if (createParticle->m_state == true){
			timer1 -= dt;
			if (Mouse().isButtonPressed(Mouse::Button::Left) && timer1<= 0){
				shared_ptr<Particle> pk = make_shared<Particle>(1.0f, 0.0f, 1000.0f, 15.0f, MousePos(actualView, w));
				if (toggleImmobile->m_state) {pk->immobile = true; pk->shape.setFillColor(Color::Yellow);}
				particles.push_back(pk);
				createParticle->onClick();
				timer1 = .15f;
			}
		}
		
		/// Stick creation logic
		if (selectedParticle1 != nullptr){
			Vector2f mousePos = MousePos(actualView, w);
			tempStick.setSize(Vector2f(5.0f, distance(selectedParticle1->getPosition(), mousePos)));
			tempStick.setPosition(selectedParticle1->getPosition());
			float angleBetweenPoints = atan2(mousePos.y - selectedParticle1->getPosition().y, mousePos.x - selectedParticle1->getPosition().x)*180/PI - 90;
			tempStick.setRotation(angleBetweenPoints);
			
			for (auto pk : particles){
				if (selectedParticle1 == pk) continue;
				if (distance(mousePos, pk->getPosition()) < pk->m_radius){
					tempStick.setSize(Vector2f(5.0f, distance(selectedParticle1->getPosition(), mousePos)));
					tempStick.setRotation(angle(selectedParticle1->getPosition(), pk->getPosition()));
					if (Mouse().isButtonPressed(Mouse::Button::Left)){
						Stick sk(selectedParticle1, pk);
						/// If spring is toggled on, the coefficient is set to a very low number
						if (toggleSpring->m_state) {sk.springCoefficient = 0.001f; sk.line.setFillColor(Color(148, 0, 211));}
						sticks.push_back(sk);
						createStick->onClick();
					}
				}
			}
			if (createStick->m_state == false){
				tempStick.setSize(Vector2f(0.0f, 0.0f));
				selectedParticle1 = nullptr;
			}
		}
		if (createStick->m_state == true){
			for (auto pk : particles){
				if (Mouse().isButtonPressed(Mouse::Button::Left) && distance(MousePos(actualView, w), pk->getPosition()) < pk->m_radius){
					selectedParticle1 = pk;
				}
			}
			
		}
		
		/// Erasing
		if (Mouse().isButtonPressed(Mouse::Button::Right)){
			Vector2f mousePos = MousePos(actualView, w);
			for (int i = 0; i<particles.size(); ++i){
				if (particles[i]->shape.getGlobalBounds().contains(mousePos)){
					vector<int>indexes;
					for (int j = 0; j<sticks.size(); ++j){
						if (sticks[j].contains(particles[i])){
							indexes.push_back(j);
						}
					}
					for (int j = 0; j<indexes.size(); ++j){
						sticks.erase(sticks.begin()+indexes[j]);
					}
					particles.erase(particles.begin()+i);
					break;
				}
			}
			for (int i = 0; i<sticks.size(); ++i){
				if (sticks[i].line.getGlobalBounds().contains(mousePos)){
					sticks.erase(sticks.begin()+i);
					break;
				}
			}
		}
		if (clear->clicked){
			particles.clear();
			sticks.clear();
			clear->clicked = false;
		}
		///
		
		/// Camera control, this can be improved as well but it works
		if (Mouse().isButtonPressed(Mouse::Button::Middle)){
			Vector2f newPos = MousePos(actualView, w) - actualView.getCenter();
			actualView.setCenter(newPos);
		}
		
		
		/// Drawing
		w.clear(Color(50, 50, 50, 50));
		w.setView(actualView);
		
		/// Background squares, uncomment if you want to see it, I'm aware this could be done a lot clever, in a future version may I polish this up
//		for (RectangleShape &s : squares){
//			w.draw(s);
//		}
		w.draw(tempStick);
		for (Stick &sk : sticks){
			sk.draw(w);
		}
		for (auto pk : particles){
			pk->draw(w);
		}
		w.setView(guiView);
		for (auto &bk : buttons){
			bk->draw(w);
		}
		w.display();
	}
	return 0;
}

/// Func for avoiding pollution in inicialization
void setupBackgroundSquares(vector<RectangleShape>&squares){
	float squareSize = 70.0f;
	float squareThickness = 5.0f;
	for (int i = 0; i<25; ++i){
		
		for (int j = 0; j<20; ++j){
			RectangleShape r;
			r.setSize(Vector2f(squareSize, squareSize));
			r.setOrigin(squareSize/2.0f, squareSize/2.0f);
			r.setPosition(squareSize*i, squareSize*j);
			r.setFillColor(Color::Transparent);
			r.setOutlineColor(Color(15, 15, 15, 50));
			r.setOutlineThickness(squareThickness);
			squares.push_back(r);
		}
	}
}
	
Vector2f MousePos(View &actualView, RenderWindow &w){
	Vector2i pixelPos = Mouse::getPosition(w);
	Vector2f worldPos = w.mapPixelToCoords(pixelPos, actualView);
}

