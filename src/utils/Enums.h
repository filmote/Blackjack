#pragma once

#include <stdint.h>
#include "Utils.h"

#define _DEBUG
#define SOUND_ON_OFF
#define SOUND

#ifdef SOUND
extern uint8_t hpISR;
#endif

// ------------------------------------------------------------------------------------------------

//#define DEBUG_DEALER_BLACKJACK_10_A
//#define DEBUG_DEALER_BLACKJACK_A_10
//#define DEBUG_DEALER_PAIR_10
#define DEBUG_DEALER_LOW_HAND

//#define DEBUG_PLAYER_INIT_BLACKJACK_10_A
//#define DEBUG_PLAYER_INIT_BLACKJACK_A_10
#define DEBUG_PLAYER_INIT_PAIR_10
//#define DEBUG_PLAYER_INIT_PAIR_A
//#define DEBUG_PLAYER_INIT_LOW_HAND
#define DEBUG_PLAYER_SPLIT_FIRST_A
//#define DEBUG_PLAYER_SPLIT_FIRST_10
#define DEBUG_PLAYER_SPLIT_SECOND_A
//#define DEBUG_PLAYER_SPLIT_SECOND_10
//#define DEBUG_PLAYER_PLAY_FIRST_A
//#define DEBUG_PLAYER_PLAY_FIRST_10
//#define DEBUG_PLAYER_PLAY_SECOND_A
//#define DEBUG_PLAYER_PLAY_SECOND_10

// ------------------------------------------------------------------------------------------------


constexpr const static uint8_t FLASH_DELAY = 32;
constexpr const static uint8_t NO_VALID_BUTTON = 255;
constexpr const static uint8_t STARTING_PURSE = 100;

constexpr const static uint8_t CARD_LARGE_SPACING = 12;
constexpr const static uint8_t CARD_LARGE_SPACING_DEALER = 10;
constexpr const static uint8_t CARD_LARGE_SPACING_FULL = 22;
constexpr const static uint8_t CARD_LARGE_SPACING_ROTATED = 9;
constexpr const static uint8_t CARD_HAND_SPACING = 12;
constexpr const static uint8_t CARD_DEALER_CENTER = 40;
constexpr const static uint8_t CARD_PLAYER_CENTER = 64;
constexpr const static uint8_t CARD_LARGE_TOP_PLAYER = 32;
constexpr const static uint8_t CARD_SMALL_TOP_PLAYER = 37;
constexpr const static uint8_t CARD_LARGE_TOP_DEALER = 0;

constexpr const static uint8_t HIGHLIGHT_BUTTON_DO_NOT_CHANGE = 255;

enum class MessageNumber : uint8_t {
  None = 0,
  BustFirstHand,
  BustOnlyHand,
  BustSecondHand,
  BothHaveBlackjack,
  DealerHasBlackjack,
	DealerNoBlackjack,
  DealerHasBlackjackWithInsurance,
  PushOnBlackjack,
  OnlytHandWinner,
  OnlyHandLoser,
  OnlyHandPush,
  OnlyHandBlackjack,
  FirstHandWinner,
  FirstHandLoser,
  FirstHandPush,
  FirstHandBlackjack,
  SecondHandWinner,
  SecondHandLoser,
  SecondHandPush,
  SecondHandBlackjack
};

char const messageText_01[] PROGMEM = "First~hand~is~bust!";
char const messageText_02[] PROGMEM = "Bust!";
char const messageText_03[] PROGMEM = "Second~hand~is~bust!";
char const messageText_04[] PROGMEM = "Two~blackjacks!";
char const messageText_05[] PROGMEM = "Dealer~has~Blackjack!";
char const messageText_06[] PROGMEM = "Dealer~has~nothing!";
char const messageText_07[] PROGMEM = "Insured Blackjack!";
char const messageText_08[] PROGMEM = "Push~on~two~Blackjacks!";
char const messageText_09[] PROGMEM = "Your~hand~wins!";
char const messageText_10[] PROGMEM = "Your~hand~loses!";
char const messageText_11[] PROGMEM = "Your~hand~pushes!";
char const messageText_12[] PROGMEM = "You~have~Blackjack!";
char const messageText_13[] PROGMEM = "First~hand~wins!";
char const messageText_14[] PROGMEM = "First~hand~loses!";
char const messageText_15[] PROGMEM = "First~hand~pushes!";
char const messageText_16[] PROGMEM = "First~hand~has~Blackjack!";
char const messageText_17[] PROGMEM = "Second~hand~wins!";
char const messageText_18[] PROGMEM = "Second~hand~loses!";
char const messageText_19[] PROGMEM = "Second~hand~pushes!";
char const messageText_20[] PROGMEM = "Second~hand~has~Blackjack!";

char const * const messageTexts[] = {
	messageText_01,
	messageText_02,
	messageText_03,
	messageText_04,
	messageText_05,
	messageText_06,
  messageText_07,
  messageText_08,
  messageText_09,
  messageText_10,
  messageText_11,
  messageText_12,
  messageText_13,
  messageText_14,
  messageText_15,
  messageText_16,
  messageText_17,
  messageText_18,
  messageText_19,
  messageText_20,
};


enum class ButtonDisplay: uint8_t {
  DoNotChange,
  None,
  BetButtons,
  Options,
  Insurance,
  EndOfGame,
  OptionsDisabled,
  GamePlay,
  GamePlayDisabled,
  OKOnly
};

enum class Turn : uint8_t {
  Player,
  Dealer
};

enum class Hand : uint8_t {
  First,
  Second,
	Dealer,
};

enum class Buttons : uint8_t {
  EndOfGame_Continue = 0,
  EndOfGame_Quit = 1,
  PlayHand_Hit = 0,
  PlayHand_Stand,
  PlayHand_Double,
  PlayHand_Split,
	InitBet_1 = 0,
  InitBet_5,
  InitBet_10,
  InitBet_25,
  InitBet_PlayGame,
  InitBet_Clear,
	InsuranceBet_1 = 0,
  InsuranceBet_5,
  InsuranceBet_10,
  InsuranceBet_25,
  InsuranceBet_PlayGame,
  InsuranceBet_Clear,
	PeekOnTen_Continue = 0,
	PeekOnTen_Quit,
};

enum class GameStateType : uint8_t {
  None,
  PlayGame,
	SplashScreen,
	TitleScreen,
};

enum class WinStatus : uint8_t {
  None,
  Win,
  Lose,
  Push,
};

struct PlayerHand {

  bool stand = false;
  bool doubleUp = false;
  bool bust = false;
  uint8_t cardCount = 0;
  uint8_t cards[12];
  uint16_t bet = 0;

  void reset() {

    stand = false;
    doubleUp = false;
    bust = false;
    cardCount = 0;
    bet = 0;

    for (uint8_t i = 0; i < 12; i++) {

      cards[i] = 0;  

    }

  }

  bool cardIsAce(uint8_t index) {

    return (cards[index] % 13 == 0);
  }

  bool isBlackjack() {

    return (cardCount == 2 && ((cards[0] % 13 == 0 && cards[1] % 13 >= 9) || (cards[1] % 13 == 0 && cards[0] % 13 >= 9)));

  }

};

struct Player {

  PlayerHand firstHand;
  PlayerHand secondHand;
  uint16_t purse = STARTING_PURSE;

  bool split = false;

  void reset() {
  
    purse = STARTING_PURSE;
    split = false;
    firstHand.reset();
    secondHand.reset();

  }

  void resetHand() {

    firstHand.reset();
    secondHand.reset();

  }

  bool canSplit() {

    return firstHand.cardCount == 2 && secondHand.cardCount == 0 && firstHand.cards[0] % 13 == firstHand.cards[1];

  }

  bool hasSecondHand() {

    return secondHand.cardCount > 0;

  }
  
};

struct Dealer {
  
  uint8_t cardCount = 0;
  uint8_t cards[12];

  void reset() {

    cardCount = 0;
    resetHand();

  }

  void resetHand() {

    for (uint8_t i = 0; i < 12; i++) {

      cards[i] = 0;  

    }

  }

  bool cardIsAce(uint8_t index) {

    return (cards[index] % 13 == 0);
  }

  bool isBlackjack() {

    return (cardCount == 2 && ((cards[0] % 13 == 0 && cards[1] % 13 >= 9) || (cards[1] % 13 == 0 && cards[0] % 13 >= 9)));
    
  }

};

struct GameStats {

  uint16_t gamesPlayed = 0;
  uint16_t gamesWon = 0;
  uint16_t gamesLost = 0;
  uint16_t gamesPush = 0;

  void reset() {
    
    gamesPlayed = 0;
    gamesWon = 0;
    gamesLost = 0;
    gamesPush = 0;

  }    

};

struct HighlightEndOfGame {

  WinStatus status;
  Hand hand;
  int16_t win;
  int16_t loss;
  int16_t purseInc; 
  MessageNumber messageId;
  uint8_t counter;
  uint8_t changeScore;

  void reset() {
    
    status = WinStatus::None;
    win = 0;
    loss = 0;
    purseInc = 0;
    messageId = MessageNumber::None;

  }

  void setCounter(uint8_t value) {

    counter = value;//FLASH_DELAY * 4;
    changeScore = counter / 2;

  }
  
};