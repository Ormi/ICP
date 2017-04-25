/* xTamas01 and xVasko12 presents the ultimate survival Klondike guide
 * Project: ICP17, a Klondike (Solitaire) game
 * FIT VUT Brno
 * Authors: xtamas01 - Tamaškovič Marek
 *          xvasko12 - Vaško Martin
 */

#include "classes.h"
#include "core.h"
#include "color.h"

extern session_t currentSession;

Card::Card(): 	number(0),
				color(Heart),
				visible(false){}

Card::Card(int num, Color clr):	number(num),
								color(clr),
								visible(false){}

Card::Card(int num, Color clr, bool vis):	number(num),
											color(clr),
											visible(vis){}								

Card::Card(const Card &c): 	number(c.number),
							color(c.color),
							visible(c.visible){}

void Card::printColor(int num){
	switch(this->color){
	case 0:
		std::cout << BOLDRED << num << " H" << RESET << ",  ";
		break;
	case 1:
		std::cout << BOLDBLACK << num << " S" << RESET << ",  ";
		break;
	case 2:
		std::cout << BOLDRED << num << " D" << RESET << ",  ";
		break;
	case 3:
		std::cout << BOLDBLACK << num << " C" << RESET << ",  ";
		break;
	}
}

void Card::printColor(std::string str){
	switch(this->color){
	case 0:
		std::cout << BOLDRED << str << " H" << RESET << ",  ";
		break;
	case 1:
		std::cout << BOLDBLACK << str << " S" << RESET << ",  ";
		break;
	case 2:
		std::cout << BOLDRED << str << " D" << RESET << ",  ";
		break;
	case 3:
		std::cout << BOLDBLACK << str << " C" << RESET << ",  ";
		break;
	}
}

void Card::printCard(){
	Symbol sym;
	std::string tmp;
	if (this->visible == true){
		switch(this->number){
		case 1:
			sym = A;
			tmp = "A";
			break;
		case 11:
			sym = J;
			tmp = "J";
			break;
		case 12:
			sym = Q;
			tmp = "Q";
			break;
		case 13:
			sym = K;
			tmp = "K";
			break;
		default:
			sym = undef;
		}
		if (sym != undef){
			this->printColor(tmp);
		}
		else
			this->printColor(this->number);
	}
	else
		std::cout << "( # ),  ";
}

// TODO extern counter wether game could be finished
void Card::changeVisibility(){
	this->visible = !this->visible;
}

bool Card::operator==(const Card &rCard){
	return this->number == rCard.number && this->color == rCard.color;
}

Deck::Deck(): 	deck(0),
				cards(),
				position(starterDeck),
				permissions(insert_get){}

Deck::Deck(Permissions perm, Position pos, int num): deck(num),
													 cards(),
													 position(pos),
													 permissions(perm){}

Deck::Deck(const Deck &d): 	deck(d.deck),
							cards{d.cards},
							position{d.position},
							permissions{d.permissions}{}

/* check validity between two decks */
int Deck::checkValidity(Card &card){
	if ( this->permissions == insert_get && card.visible == true){
		// moving card to this , function to compare used
		if (this->position == starterDeck){
			if (this->cards.back().visible == false ||
				this->cards.back() == card)
				return -1;
			if (cardCondition(this->cards.back(), card) == true){
				return 0;
			}
		}
		// every final deck has exactly 1 color and numbered from 1 to 13
		else if (this->position == finalDeck){
			if (this->cards.empty()){
				if (card.number == 1){
					return 0;
				}
				else
					return-1;
			}
			else if ((card.number - 1) == this->cards.front().number){
				if (card.color == this->cards.front().color){
					return 0;
				}
			}
		}
	}

	return -1;
}

void Deck::pushCardVector(std::vector<Card> vect){
	for (unsigned i = 0; i != vect.size();){
		this->insertCard(vect.front());
		vect.erase(vect.begin());
	}
}

int Deck::moveCards(std::vector<Card> vect){
	int result = -1;
	if (vect.size() != 0){
		result = this->checkValidity(vect.back());
		if (result == 0){
			this->pushCardVector(vect);
		}
	}
	return result;
}

Card &Deck::getLastCard(){
	return this->cards.back();
}

int Deck::dequeue(Deck *other){
	if (this->permissions == insert && other->permissions == get){
		/* flip to flop deck */
		if (this->cards.empty()){
			Card *c = new Card(other->cards.back());
			currentSession.slot[currentSession.currentGame]->
				history.push_back({11, 12, c});

			for (unsigned i = 0; i != other->cards.size();){
				// Again from start
				other->cards.front().changeVisibility();
				this->cards.push_back(other->cards.front());
				other->cards.erase(other->cards.begin());
			}

			return 0;
		}
		else{

			other->cards.push_back(this->cards.front());
			// to flop, change visibility
			other->cards.back().changeVisibility();
			this->cards.erase(this->cards.begin());

			Card *c = new Card(other->cards.back());
			currentSession.slot[currentSession.currentGame]->
				history.push_back({12, 11, c});

			return 0;
		}
	}
	return -1;
}

void Deck::printDeck(){
	std::cout << this->deck << " ";
	// if (this->permissions == insert || this->permissions == get){
	// 	std::cout << "Deck {  ";
	// 	if (this->cards.size() > 0)
	// 		this->cards[this->cards.size()-1].printCard();
	// 	std::cout << "}\n";
	// }
	// else{
		std::cout << "Deck {  ";
		for (unsigned i = 0; i < this->cards.size(); i++){
			this->cards[i].printCard();
		}
		std::cout << "\b\b\b}\n";
	// }
}

/***************************************************/
/************ Game class implementation ************/
/***************************************************/
int Game::current_count = 0;

Game::Game(	const Game &G) : history(), mainDeck(), decks(){
	for(int i = 0; i < 13; i++){
		decks[i] = G.decks[i];
	}
}

Game& Game::operator=(const Game &G){
	if (this != &G) { // self-assignment check expected
		std::copy(G.history.begin(), G.history.end(),
              std::back_inserter(this->history));

		std::copy(G.mainDeck.begin(), G.mainDeck.end(),
              std::back_inserter(this->mainDeck));

		for(int i = 0; i < 13; i++){
			this->decks[i] = G.decks[i];
		}
    }
    return *this;
}

/* TODO repair shuffle */
Game::Game() :  history(), mainDeck(), decks(){
	current_count++;
	id = current_count;
	position = getPosition();
	
}

Game::~Game(){
	current_count--;
	for(int i = 0; i < 13; i++){
		delete(this->decks[i]);
	}
}

/* TODO add JSON library, convert - history deck id position - into json saved
 * loaded format check for permissions and some other stuff get the correct way
 * of saving  loading methods of Game class */
int Game::save(){
	return 0;
}

int Game::load(){
	return 0;
}

void Game::showGame(){
	for (int i = 12; i != -1; --i){
		if (i == 10 || i == 3)
			std::cout << "\n";
		this->decks[i]->printDeck();
	}
}

// TODO dokoncit
Move* Game::hint(){
	Card cardFrom, cardTo;
	int result;
	Move *hintMove = nullptr;

	for (int i = 0; i < 12; ++i){
		if (!this->decks[i]->cards.empty())
			cardFrom = this->decks[i]->cards.back();
		if (!this->decks[i+1]->cards.empty())
			cardTo = this->decks[i+1]->cards.back();

		if (cardFrom.number == 0)
			continue;
		// FIXME go trough all visible cards in one deck
		for (int j = 0;j < 12;++j){
			result = this->decks[j]->checkValidity(cardFrom);
			if (result == 0){
				Card *store = new Card(cardFrom);
				hintMove = new Move({i,j,store});
				break;
			}
		}
		if (hintMove != nullptr)
			return hintMove;
	}

	if (this->decks[12]->cards.empty()){
		if (this->decks[11]->cards.empty())
			return hintMove;
		hintMove = new Move({12,11,nullptr});
	}
	return hintMove;
}

bool cardCondition(Card &other, Card &card){
	if (card.number ==
		(other.number - 1)){

		if((other.color == Heart ||
			other.color == Diamond) &&
			(card.color == Spade ||
			card.color == Club)){
			return true;
		}
		else if ((other.color == Spade ||
				other.color == Club) &&
				(card.color == Diamond ||
				card.color == Heart)){
			return true;
		}
	}
	return false;
}

void printMove(std::vector<Move> mov){
	std::cout << "Move:\n";

	std::string from = "";
	std::string to = "";
	for (unsigned i = 0; i < mov.size(); ++i){
		// std::cout << mov[i].from << "to: " << mov[i].to;
		if (mov[i].from == 12)
			from = "From - Flip deck: ";
		if (mov[i].to == 12)
			to = "To - Flip deck: ";
		if (mov[i].from == 11)
			from = "From - Flop deck: ";
		if (mov[i].to == 11)
			to = "To - Flop deck";

		if (mov[i].from < 10 && mov[i].from > 3)
			from = "From - Starter deck: ";
		if (mov[i].to < 10 && mov[i].to > 3)
			to = "To - Starter deck: ";
		if (mov[i].from < 3)
			from = "From - Final deck: ";
		if (mov[i].to < 3)
			to = "To - Final deck: ";

		std::cout << from << "\n";
		std::cout << to << "\n";
		mov[i].card->printCard();
		std::cout << "\n";
	}
	std::cout << "End\n";
}

void clearHistory(std::vector<Move> mv){
	for(unsigned i = 0; i < mv.size(); ++i){
		delete mv[i].card;
	}
}

std::vector<Card> parseCard(std::string str, int *deckNumber){
	int num = 0;
	int check;
	std::vector<Card> vect;
	if (str[str.length() - 1] == ')'){
		str.erase(str.length() - 1);
	}
	check = sscanf(str.c_str(),"%d",&num);
	if (check != 1)
		return vect;
	std::string tmp = std::to_string(num);
	std::size_t pos = str.find(tmp);

	Color clr = Heart;
	str = str.substr(pos + tmp.length());
	int tmpNum = str[0];

	Card *c = nullptr;
	switch (tmpNum){
	case 'H':
		clr = Heart;
		break;
	case 'S':
		clr = Spade;
		break;
	case 'D':
		clr = Diamond;
		break;
	case 'C':
		clr = Club;
		break;
	default:
		return vect;
		break;
	}
	if (!(num >= 1 && num <= 13)){
		std::cout << "number : " << num << "\n";
		return vect;
	}
	c = new Card(num, clr);
	(*c).changeVisibility();

	int curr = currentSession.currentGame;
	Card tmpCard;
	bool found = false;

	for (int i = 0; i < 12; ++i){
		for (unsigned j = 0; j < currentSession.slot[curr]->decks[i]->cards.size(); ++j){
			tmpCard = currentSession.slot[curr]->decks[i]->cards[j];

			if (found == false){
				if (tmpCard.visible == true && tmpCard == *c){
					*deckNumber = i;
					delete (c);
					c = new Card(tmpCard);
					found = true;
					vect.push_back(std::move(tmpCard));
				}
			}
			else{
				if (tmpCard.visible == false){
					found = false;
					break;
				}
				c = checkNext(tmpCard,*c);
				if (c != nullptr)
					vect.push_back(*c);
				found = false;
			}
		}
		if (found == true)
			break;
	}

	// FIXME one more loop
	// 	else{
	// 	if (tmpCard.visible == false){
	// 		found = false;
	// 		break;
	// 	}
	// 	c = checkNext(tmpCard,*c);
	// 	if (c != nullptr)
	// 		vect.push_back(*c);
	// 	found = false;
	// }

	delete(c);
	return vect;
}

Deck *parseDeck(std::string str){
	int num = std::stoi(str);
	Deck *deck = nullptr;
	int curr = currentSession.currentGame;

	if(num > -1 && num < 13)
		deck = currentSession.slot[curr]->decks[num];
	return deck;
}


Card *checkNext(Card &other, Card &main){
	bool ret = false;
	other.printCard();
	main.printCard();

	ret = cardCondition(main,other);
	if (ret == true)
		return &main;
	return nullptr;
}

// TODO need to change visibility if is swaped card, ADD this thing to history
void undo(std::vector<Move> moveSession){
	printMove(moveSession);
	moveSession.back().card->printCard();
}