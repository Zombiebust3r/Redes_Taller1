#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>


#define MAX_MENSAJES 25


int main()
{
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::TcpSocket socket;
	char connectionType, mode;
	char buffer[2000];
	std::size_t received;
	std::string text = "Connected to: ";
	int ticks = 0;

	std::cout << "Enter (s) for Server, Enter (c) for Client: ";
	std::cin >> connectionType;
	sf::Socket::Status st;
	if (connectionType == 's')
	{
		sf::TcpListener listener;
		listener.listen(5000);
		st = listener.accept(socket);
		text += "Server";
		mode = 's';
		listener.close();
	}
	else if (connectionType == 'c')
	{
		do {
			ticks++;
			st = socket.connect(ip, 5000, sf::seconds(5.f));
			if(st != sf::Socket::Status::Done) std::cout << "NO SE PUDO CONECTAR PENDEJO TRAS 5s" << std::endl;
		} while (st != sf::Socket::Status::Done && ticks < 2);
		
		text += "Client";
		mode = 'r';

	}
	if (st == sf::Socket::Status::Done) {
		socket.send(text.c_str(), text.length() + 1);
		socket.receive(buffer, sizeof(buffer), received);

		std::cout << buffer << std::endl;
	}
	

	bool done = false;
	while (!done && (st == sf::Socket::Status::Done))
	{

		std::vector<std::string> aMensajes;

		sf::Vector2i screenDimensions(800, 600);

		sf::RenderWindow window;
		window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

		sf::Font font;
		if (!font.loadFromFile("courbd.ttf"))
		{
			std::cout << "Can't load the font file" << std::endl;
		}

		sf::String mensaje = " >";

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
					else if (evento.key.code == sf::Keyboard::Return)
					{
						aMensajes.push_back(mensaje);
						if (aMensajes.size() > 25)
						{
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
						}
						mensaje = ">";
					}
					break;
				case sf::Event::TextEntered:
					if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
						mensaje += (char)evento.text.unicode;
					else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
						mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
					break;
				}
			}
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
	}

}