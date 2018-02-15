#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <time.h>


#define MAX_MENSAJES 25

std::vector<std::string> aMensajes;
std::mutex myMutex;
bool connected = false;
std::thread receiveThread;
sf::IpAddress ip = sf::IpAddress::getLocalAddress();
sf::TcpSocket socket;
char connectionType, mode;
char buffer[2000];
std::size_t received;
std::string text = "Connected to: ";
int ticks = 0;
std::string windowName;
sf::Socket::Status st;

//Hacer que cliente y servidor tengan mensajes de diferente color
struct Message {
	std::string s;
	sf::Color textColor;
};

void addMessage(std::string s) {
	std::lock_guard<std::mutex> guard(myMutex);
	aMensajes.push_back(s);
	if (aMensajes.size() > 25) {
		aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
	}
}

void receiveFunction(sf::TcpSocket* socket, bool* _connected) {
	char receiveBuffer[2000];
	std::size_t _received;
	while (*_connected) {
		socket->receive(receiveBuffer, sizeof(receiveBuffer), _received);
		if (_received > 0) {
			addMessage(receiveBuffer);
			if (strcmp(receiveBuffer, " >exit") == 0) {
				std::cout << "EXIT" << std::endl;
				*_connected = false;
				addMessage("DISONNECTED FROM CHAT");
			}
		}
	}
}

void nonBlockedComunication() {
	if (st == sf::Socket::Status::Done) {
		socket.send(text.c_str(), text.length() + 1);
		socket.receive(buffer, sizeof(buffer), received);

		std::cout << buffer << std::endl;
		connected = true;

		std::cout << buffer << std::endl;
	}

}

void blockeComunication() {

	receiveThread = std::thread(receiveFunction, &socket, &connected);

	bool done = false;
	while (!done && (st == sf::Socket::Status::Done) && connected)
	{

		sf::Vector2i screenDimensions(800, 600);

		sf::RenderWindow window;
		window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), windowName);

		sf::Font font;
		if (!font.loadFromFile("courbd.ttf"))
		{
			std::cout << "Can't load the font file" << std::endl;
		}

		std::string mensaje = " >";

		sf::Text chattingText(mensaje, font, 14);

		chattingText.setFillColor(sf::Color(0, 160, 0));
		chattingText.setStyle(sf::Text::Bold);


		sf::Text text(mensaje, font, 14);
		text.setFillColor(sf::Color(0, 160, 0));
		text.setStyle(sf::Text::Bold);
		text.setPosition(0, 560);

		sf::RectangleShape separator(sf::Vector2f(800, 5));
		separator.setFillColor(sf::Color(200, 200, 200, 255));
		separator.setPosition(0, 550);

		while (window.isOpen())
		{
			sf::Event evento;
			while (window.pollEvent(evento))
			{
				switch (evento.type)
				{
				case sf::Event::Closed:
					//DISCONECT FROM SERVER
					done = true;
					window.close();
					break;
				case sf::Event::KeyPressed:
					if (evento.key.code == sf::Keyboard::Escape)
						window.close();
					else if (evento.key.code == sf::Keyboard::Return) //envia mensaje
					{
						char* jaja = "";
						socket.send(mensaje.c_str(), mensaje.length() + 1);
						addMessage(mensaje);
						if (strcmp(mensaje.c_str(), " >exit") == 0) {
							std::cout << "EXIT" << std::endl;
							addMessage("YOU DISCONNECTED FROM CHAT");
							connected = false;
							done = true;
							window.close();
						}
						mensaje = " >";
					}
					break;
				case sf::Event::TextEntered:
					if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
						mensaje += (char)evento.text.unicode;
					else if (evento.text.unicode == 8 && mensaje.length() > 0)
						mensaje.erase(mensaje.length() - 1, mensaje.length());
					break;
				}
			}
			/*if (!serv)
			{
			receiveFunction(&socket);
			serv = true;
			socket.receive(buffer, sizeof(buffer), received);
			if (received > 0)
			{
			std::cout << "Received: " << buffer << std::endl;
			aMensajes.push_back(buffer);
			serv = true;
			if (strcmp(buffer, " >exit") == 0)
			{
			//Desconectar
			done = true;
			break;
			}
			}
			}*/


			window.draw(separator);
			for (size_t i = 0; i < aMensajes.size(); i++)
			{
				std::string chatting = aMensajes[i];
				chattingText.setPosition(sf::Vector2f(0, 20 * i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}
			std::string mensaje_ = mensaje + "_";
			text.setString(mensaje_);
			window.draw(text);


			window.display();
			window.clear();
		}
		receiveThread.join();
	}
}

int main()
{
	srand(time(NULL));
	sf::Color color(rand() % 255 + 0, rand() % 255 + 0, rand() % 255 + 0, 255);

	bool serv;
	std::string serverMode;

	std::cout << "Enter (s) for Server, Enter (c) for Client: ";
	std::cin >> connectionType;

	if (connectionType == 's')
	{
		serv = true;
		windowName = "Server Chat Window";
		std::cout << "Enter (b) for Blocking, Enter (n) for NonBlocking: ";
		std::cin >> serverMode;

		sf::TcpListener listener;
		listener.listen(5000);
		if (strcmp(serverMode.c_str(), "b") == 0) st = listener.accept(socket);
		else if (strcmp(serverMode.c_str(), "n") == 0) {
			listener.setBlocking(false);
			while (true) {
				st = listener.accept(socket);
				if (st == sf::Socket::Status::Done) {
					break;
				}
				else if (st == sf::Socket::Status::NotReady) {
					std::cout << "NOT READY" << std::endl;
				}
			}
		}

		text += "Server";
		mode = 's';
		listener.close();
	}
	else if (connectionType == 'c')
	{
		serv = false;
		do {
			ticks++;
			st = socket.connect(ip, 5000, sf::seconds(5.f));
			if (st != sf::Socket::Status::Done) std::cout << "NO SE PUDO CONECTAR PENDEJO TRAS 5s" << std::endl;
		} while (st != sf::Socket::Status::Done && ticks < 2);

		text += "Client";
		mode = 'r';
		windowName = "Client Chat Window";

	}

	if (st == sf::Socket::Status::Done) {
		socket.send(text.c_str(), text.length() + 1);
		socket.receive(buffer, sizeof(buffer), received);

		std::cout << buffer << std::endl;
		connected = true;

		if (connectionType == 'c') {
			//Recibir tipo de conexión:
			socket.receive(buffer, sizeof(buffer), received);
			serverMode = buffer;
		} else if (connectionType == 's') {
			//Enviar tipo de conexión:
			socket.send(serverMode.c_str(), serverMode.length() + 1);
		}

		if (strcmp(serverMode.c_str(), "b") == 0) { blockeComunication(); }
		else if (strcmp(serverMode.c_str(), "n") == 0) { std::cout << "NON BLOCKING" << std::endl; system("pause"); }
	}

}