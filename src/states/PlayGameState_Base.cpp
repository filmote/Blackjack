#include "PlayGameState.h"
#include "../images/Images.h"
#define xDEBUG_CASE

static const uint8_t BETS[] = { 1, 5, 10, 25 };

// ----------------------------------------------------------------------------
//  Initialise state ..
//
void PlayGameState::activate(StateMachine & machine) {
  
  auto & gameStats = machine.getContext().gameStats;

  player.reset(); 
  dealer.reset();
  gameStats.reset();

	this->counter = 0;	
  this->handInPlay = Hand::First;
	this->flashDetails = false;
  this->insurance = 0;

  this->currentBetInit = 0;
  this->currentBetTotal = 0;
  this->currentWin = 0;

	changeView(machine, ViewState::StartHand);

}


// ---------------------------------------------------------------------------------------------------------------------------
//  Handle state updates .. 
// ---------------------------------------------------------------------------------------------------------------------------
//
void PlayGameState::update(StateMachine & machine) {

  auto & arduboy = machine.getContext().arduboy;
  auto & gameStats = machine.getContext().gameStats;
  auto justPressed = arduboy.justPressedButtons();

	switch (this->viewState) {

		case ViewState::StartHand:
Serial.println(F("-----------------------------------------"));    
#ifdef DEBUG_CASE
Serial.println(F("StartHand "));
#endif
      dealer.reset();
			this->endOfHand = false;
			this->buttonMode = ButtonDisplay::BetButtons;
			this->flashDetails = false;
			
			this->handInPlay = Hand::First;
			this->dealer.resetHand();
			this->player.resetHand();
			this->insurance = 0;
			this->currentBetInit = 0;
			this->currentBetTotal = 0;
			this->currentWin = 0;

  		for (uint8_t x = 0; x < 52; x++) {
				
				cards[x] = 0;
				
			}

      changeView(machine, ViewState::InitBet);
      // fall through intentional

		case ViewState::InitBet:
#ifdef DEBUG_CASE
Serial.println(F("InitBet "));
#endif
			if (justPressed & LEFT_BUTTON) 	{ this->highlightedButton = decreaseHighlightButton_BettingButtons(this->highlightedButton); }
			if (justPressed & RIGHT_BUTTON) { this->highlightedButton = increaseHighlightButton_BettingButtons(this->highlightedButton); }
			if (justPressed & A_BUTTON) 		{ 
				
				switch (static_cast<Buttons>(this->highlightedButton)) {
					
					case Buttons::InitBet_1 ... Buttons::InitBet_25: 
						this->currentBetInit = this->currentBetInit + BETS[this->highlightedButton];
						this->currentBetTotal = this->currentBetInit;
						player.firstHand.bet = this->currentBetInit;
						player.purse = player.purse - BETS[this->highlightedButton];
						break;
					
					case Buttons::InitBet_PlayGame: 
						changeView(machine, ViewState::InitDeal, 0, ButtonDisplay::None);
						break;

					case Buttons::InitBet_Clear:
						player.purse = player.purse + currentBetInit;
						this->currentBetInit = 0;
						this->currentBetTotal = 0;
						break;

					default: break;

				}

				this->highlightedButton = isValidButton_BettingButtons(this->highlightedButton);

			}

			break;

		case ViewState::InitDeal:
#ifdef DEBUG_CASE
Serial.println(F("InitDeal "));
#endif
			if (arduboy.everyXFrames(15)) {

				switch (this->counter) {

					case 0:
					case 2:
						getCard(Turn::Dealer, Hand::First);
						dealer.cards[0] = 3;
//						dealer.cards[1] = 0;
						break;

					case 1:
					case 3:
						getCard(Turn::Player, Hand::First);
						this->player.firstHand.cards[0] = 9;
						this->player.firstHand.cards[1] = 9;
						break;

				}

				this->counter++;

				if (this->counter == 4) {


					// Offer insurance ?

					if (dealer.cardIsAce(1) && player.purse >= 1) {

						changeView(machine, ViewState::OfferInsurance);

					}
					else {

						changeView(machine, ViewState::PlayHand);

					}

				}

			}

			break;

		case ViewState::OfferInsurance:
#ifdef DEBUG_CASE
Serial.println(F("OfferInsurance "));
#endif
			buttonMode = ButtonDisplay::Insurance;

			if (justPressed & LEFT_BUTTON) 	{ this->highlightedButton = decreaseHighlightButton_InsuranceButtons(this->highlightedButton); }
			if (justPressed & RIGHT_BUTTON) { this->highlightedButton = increaseHighlightButton_InsuranceButtons(this->highlightedButton); }
			if (justPressed & A_BUTTON) 		{ 
				
				switch (static_cast<Buttons>(this->highlightedButton)) {
					
					case Buttons::InsuranceBet_1 ... Buttons::InsuranceBet_25: 
						this->insurance = this->insurance + BETS[this->highlightedButton];
						this->currentBetTotal = this->currentBetTotal + 1;
						player.purse = player.purse - BETS[this->highlightedButton];
						break;
					
					case Buttons::InsuranceBet_PlayGame: 
						changeView(machine, ViewState::PeekOnTen, 0, ButtonDisplay::None);
						break;

					case Buttons::InsuranceBet_Clear:
            if (this->insurance > 0) {
              player.purse = player.purse + this->insurance;
              this->insurance = 0;
              this->currentBetTotal = this->currentBetInit;
            }
            else {
              changeView(machine, ViewState::PeekOnTen, 0, ButtonDisplay::None);
            }
						break;

					default: break;

				}

				this->highlightedButton = isValidButton_InsuranceButtons(this->highlightedButton);

			}

			break;

		case ViewState::PeekOnTen:
#ifdef DEBUG_CASE
Serial.println(F("PeekOnTen "));
#endif
			if (this->counter < 32) {

				switch (this->counter) {

					case 0:
//						this->insuranceResult = InsuranceResult::Peeking;	
						this->highlightedButton = 0;
						break;

					case 1 ... 30:
						break;

					case 31:

            if (calculateHand(Turn::Dealer, true) == 21) {

              if (calculateHand(Turn::Player, Hand::First, true) == 21) {
              
//                this->insuranceResult = InsuranceResult::BothHaveBlackjack;			
                player.purse = player.purse + this->currentBetTotal + (2 * this->insurance);
                this->buttonMode = ButtonDisplay::EndOfGame;
								this->flashDetails = true;
                gameStats.gamesPush++;

								if (this->insurance == 0) {
	                // this->winStatus = WinStatus::Push;
									// this->winStatusAmount = this->currentBetInit;
                  highlightPush(Hand::First, MessageNumber::BothHaveBlackjack);
								}
								else {
	                // this->winStatus = WinStatus::Win;
									// this->winStatusAmount = (2 * this->insurance);
                  highlightWin(Hand::First, (2 * this->insurance), (2 * this->insurance),  MessageNumber::BothHaveBlackjack);
								}

              }
              else {

								this->buttonMode = ButtonDisplay::EndOfGame;
								// this->flashDetails = true;
								// this->winStatusAmount = this->currentBetInit - (this->insurance * 2);
                gameStats.gamesLost++;

								if (this->insurance == 0) {
									Serial.println(F("highlightLoss A"));	
									highlightLoss(Hand::First, -currentBetInit, MessageNumber::DealerHasBlackjack);
//									this->insuranceResult = InsuranceResult::DealerHasBlackjack;
									// this->winStatus = WinStatus::Lose;

								}
								else {

									// this->insuranceResult = InsuranceResult::DealerHasBlackjackWithInsurance;
	                // player.purse = player.purse + this->winStatusAmount;

									if (this->currentBetInit - (this->insurance * 2) == 0) {
//										this->winStatus = WinStatus::Push;
										highlightPush(Hand::First, MessageNumber::DealerHasBlackjackWithInsurance);
									}
									else {
									Serial.println(F("highlightLoss B"));	
										highlightLoss(Hand::First, -(this->currentBetInit - (this->insurance * 2)), MessageNumber::DealerHasBlackjack);
//										this->winStatus = WinStatus::Lose;
									}

								}
  
              }

            }
            else {

							if (this->insurance != 0) {
								// this->insuranceResult = InsuranceResult::DealerNoBlackjack;
								// this->winStatus = WinStatus::Lose;
								// this->flashDetails = true;
								// this->winStatusAmount = this->insurance;
								Serial.println(F("highlightLoss C"));	
								highlightLoss(Hand::First, -this->insurance, MessageNumber::DealerNoBlackjack);
								this->currentBetTotal = this->currentBetInit;
							}

							this->buttonMode = ButtonDisplay::OKOnly;

            }

						break;

				}

				this->counter++;

			}
			else {

				if (justPressed & LEFT_BUTTON && this->highlightedButton == 1) 	{ this->highlightedButton = 0; }
				if (justPressed & RIGHT_BUTTON && this->highlightedButton == 0) { this->highlightedButton = 1; }

				if (justPressed & A_BUTTON) {
					
					if (this->insuranceResult != InsuranceResult::DealerNoBlackjack) {

						switch (static_cast<Buttons>(this->highlightedButton)) {
						
							case Buttons::PeekOnTen_Continue:
								this->flashDetails = true;
								this->winStatus = WinStatus::None;
								changeView(machine, ViewState::StartHand); 
								break;

							case Buttons::PeekOnTen_Quit:
								machine.changeState(GameStateType::TitleScreen); 
								break;

							default: break;

						}

					}
					else {

            changeView(machine, ViewState::PlayHand, 0, ButtonDisplay::GamePlay);
            this->flashDetails = false;

					}

				}

			}

			break;
 
		case ViewState::PlayHand:
#ifdef DEBUG_CASE
Serial.print(F("PlayHand "));
Serial.print(justPressed);
Serial.print(F(", HIP "));
Serial.println((uint8_t)handInPlay);
#endif
      this->buttonMode = ButtonDisplay::GamePlay;

      if (justPressed & LEFT_BUTTON) 	{ this->highlightedButton = decreaseHighlightButton_GamePlayButtons(this->highlightedButton); }
			if (justPressed & RIGHT_BUTTON) { this->highlightedButton = increaseHighlightButton_GamePlayButtons(this->highlightedButton); }
			if (justPressed & A_BUTTON) 		{ 
				
				switch (static_cast<Buttons>(this->highlightedButton)) {
					
					case Buttons::PlayHand_Hit:   // Hit machine

            if (handInPlay == Hand::First) {
                
              getCard(Turn::Player, this->handInPlay);

              if (calculateHand(Turn::Player, Hand::First, false) > 21) {
                
                changeView(machine, ViewState::Bust);
              
              }
              
            }
            else {

              getCard(Turn::Player, this->handInPlay);
              
              if (calculateHand(Turn::Player, Hand::Second, false) > 21) {
                
                changeView(machine, ViewState::Bust);
              
              }
            
            }
            break;

          case Buttons::PlayHand_Stand:
            playNextHand(machine);
            break;

          case Buttons::PlayHand_Split:
          Serial.println(F("Split"));
            changeView(machine, ViewState::SplitCards);
            break;

          case Buttons::PlayHand_Double:
            changeView(machine, ViewState::DoubleUp);
            break;

					default: break;

        }

      }

			break;

    case ViewState::SplitCards:

      this->buttonMode = ButtonDisplay::None;

      if (this->counter < 3) {
#ifdef DEBUG_CASE
Serial.print(F("SplitCards "));
Serial.print(F(", HIP "));
Serial.println((uint8_t)handInPlay);
#endif          
        if (arduboy.everyXFrames(15)) {
      Serial.println(this->counter);      
          switch (this->counter) {

            case 0:
              player.firstHand.cardCount--;
              player.secondHand.cards[0] = player.firstHand.cards[1];
              player.secondHand.cardCount++;
              player.secondHand.bet = currentBetInit;
              player.purse = player.purse - currentBetInit;
              currentBetTotal = currentBetTotal + currentBetInit;
Serial.print(F(">> HIP "));
Serial.println((uint8_t)handInPlay);
              break;

            case 1:
              getCard(Turn::Player, Hand::First);
              break;

            case 2:
              getCard(Turn::Player, Hand::Second);
              break;

          }

          this->counter++;

        }
      
      }
      else {

        this->counter++;
        this->highlightedButton = 0;

        if (player.firstHand.cardIsAce(0)) {
          player.firstHand.doubleUp = true;
          player.secondHand.doubleUp = true;
          changeView(machine, ViewState::PlayDealerHand);
        }
        else {
          changeView(machine, ViewState::PlayHand);
        }   

      }

      break;

		case ViewState::DoubleUp:
#ifdef DEBUG_CASE
Serial.println(F("DoubleUp "));
#endif

      this->buttonMode = ButtonDisplay::None;

      switch (handInPlay) {

        case Hand::First:
          player.firstHand.doubleUp = true;
          player.firstHand.bet = player.firstHand.bet + currentBetInit;
          break;

        case Hand::Second:
          player.secondHand.doubleUp = true;
          player.secondHand.bet = player.secondHand.bet + currentBetInit;
          break;

        default: break;

      }

      player.purse = player.purse - currentBetInit;
Serial.print(F("getCard(Turn::Player, "));
Serial.print((uint8_t)handInPlay);      
Serial.println(F(")"));
      getCard(Turn::Player, handInPlay);
      currentBetTotal = currentBetTotal + currentBetInit;
      
      if (calculateHand(Turn::Player, handInPlay, false) > 21) {
        
        changeView(machine, ViewState::Bust);
      
      }
      else {
  
        playNextHand(machine);
    
      }   

      break;

    case ViewState::PlayDealerHand:
#ifdef DEBUG_CASE
Serial.println(F("PlayDealerHand "));
#endif

      this->buttonMode = ButtonDisplay::None;
			this->handInPlay = Hand::Dealer;

			if (arduboy.everyXFrames(15)) {

				if (calculateHand(Turn::Dealer, false) <= 16) {
					
					getCard(Turn::Dealer, Hand::First);

				}
				else {

					changeView(machine, ViewState::CheckForWins);

				}

			}

      break;

		case ViewState::CheckForWins:
#ifdef DEBUG_CASE
Serial.println(F("CheckForWins "));
#endif


      this->buttonMode = ButtonDisplay::None;


			// Are there two player hands ?
			
			if (player.secondHand.cardCount > 0) {

				if (!player.firstHand.bust && !player.secondHand.bust) {
					
					switch (this->counter) {

						case 0 ... 30: break;

						case 31:
							
							if (calculateHand(Turn::Player, Hand::First, true) > calculateHand(Turn::Dealer, true) || calculateHand(Turn::Dealer, true) > 21) {

								if (isBlackjack(Turn::Player, Hand::First)) {
									Serial.println(F("highlightWin A"));	
									highlightWin(Hand::First, player.firstHand.bet * 3 / 2, player.firstHand.bet * 5 / 2);

								}
								else {        
								
									Serial.println(F("highlightWin B"));	
									highlightWin(Hand::First, player.firstHand.bet, player.firstHand.bet * 2);
									
								}
								
							}
							else {

								Serial.println(F("highlightLoss D"));	
								highlightLoss(Hand::First, -player.firstHand.bet);
								
							}

							break;

						case 32 ... 62: break;

						case 63:
						
							if (calculateHand(Turn::Player, Hand::Second, true) > calculateHand(Turn::Dealer, true) || calculateHand(Turn::Dealer, true) > 21) {

								if (isBlackjack(Turn::Player, Hand::Second)) {

									Serial.println(F("highlightWin C"));	
									highlightWin(Hand::Second, player.secondHand.bet * 3 / 2, player.secondHand.bet * 5 / 2);

								}
								else {        
								
									Serial.println(F("highlightWin D"));	
									highlightWin(Hand::Second, player.secondHand.bet, player.secondHand.bet * 2);
									
								}
								
							}
							else {
								
								if (isBlackjack(Turn::Player, Hand::Second)) {

									Serial.println(F("highlightWin E"));	
									highlightWin(Hand::Second, player.secondHand.bet * 3 / 2, player.secondHand.bet * 5 / 2);

								}
								else {        
								
									Serial.println(F("highlightLoss E"));	
									highlightLoss(Hand::Second, -player.secondHand.bet);
									
								}
								
							}

							break;

						case 64 ... 94: break;

						case 95:
		
							changeView(machine, ViewState::EndOfGame, 0, ButtonDisplay::EndOfGame);
							break;


					}
					
				}
						
				if (!player.firstHand.bust && player.secondHand.bust) {

					switch (this->counter) {

						case 0 ... 30: break;

						case 31:

							if (calculateHand(Turn::Player, Hand::First, true) > calculateHand(Turn::Dealer, true) || calculateHand(Turn::Dealer, true) > 21) {

								if (isBlackjack(Turn::Player, Hand::First)) {
									Serial.println(F("highlightWin F"));	
									highlightWin(Hand::First, player.firstHand.bet * 3 / 2, player.firstHand.bet * 5 / 2);

								}
								else {        
									Serial.println(F("highlightWin G"));									
									highlightWin(Hand::First, player.firstHand.bet, player.firstHand.bet * 2);
									
								}        
								
							}

							break;

						case 32 ... 62: break;

						case 63:
		
							changeView(machine, ViewState::EndOfGame, 0, ButtonDisplay::EndOfGame);
							break;

					}

				}
						
				if (player.firstHand.bust && !player.secondHand.bust) {

					switch (this->counter) {

						case 0 ... 30: break;

						case 31:					

							if (calculateHand(Turn::Player, Hand::Second, true) > calculateHand(Turn::Dealer, true) || calculateHand(Turn::Dealer, true) > 21) {
				
								if (isBlackjack(Turn::Player, Hand::Second)) {
									Serial.println(F("highlightWin H"));	
									highlightWin(Hand::Second, player.secondHand.bet * 3 / 2, player.secondHand.bet * 5 / 2);

								}
								else {        
									Serial.println(F("highlightWin I"));									
									highlightWin(Hand::Second, player.secondHand.bet, player.secondHand.bet * 2);
									
								}
								
							}
							else {
								
								if (isBlackjack(Turn::Player, Hand::Second)) {
									Serial.println(F("highlightWin J"));	
									highlightWin(Hand::Second, player.secondHand.bet * 3 / 2, player.secondHand.bet * 5 / 2);

								}
								else {        
									Serial.println(F("highlightLoss F"));	
									highlightLoss(Hand::Second, -player.secondHand.bet);
									
								}
								
							}

							break;

						case 32 ... 62: break;

						case 63:
		
							changeView(machine, ViewState::EndOfGame, 0, ButtonDisplay::EndOfGame);
							break;

					}

				}
				
			}
			
			
			// Only one hand in play ..

			else {

				switch (this->counter) {

					case 0 ... 30: break;

					case 31:	
												
						if (calculateHand(Turn::Player, Hand::First, true) > calculateHand(Turn::Dealer, true) || calculateHand(Turn::Dealer, true) > 21) {
				
							if (isBlackjack(Turn::Player, Hand::First)) {
									Serial.println(F("highlightWin K"));	
								highlightWin(Hand::First, player.firstHand.bet * 3 / 2, player.firstHand.bet * 5 / 2);

							}
							else {        
									Serial.println(F("highlightWin L"));									
								highlightWin(Hand::First, player.firstHand.bet, player.firstHand.bet * 2);
									
							} 
								
						}
						else {
								
							if (isBlackjack(Turn::Player, Hand::First)) {
									Serial.println(F("highlightWin M"));	
								highlightWin(Hand::First, player.firstHand.bet * 3 / 2, player.firstHand.bet * 5 / 2);

							}
							else {        
								Serial.println(F("highlightLoss G"));	
								highlightLoss(Hand::First, -player.firstHand.bet);
								
							}
								
						}

						break;

					case 32 ... 62: break;

					case 63:
						changeView(machine, ViewState::EndOfGame);
						break;
					
				}

			}
			
			this->counter++;
			break;

    case ViewState::EndOfGame:
#ifdef DEBUG_CASE
Serial.println(F("EndOfGame "));
#endif
			this->buttonMode = ButtonDisplay::EndOfGame;

      if (justPressed & LEFT_BUTTON && this->highlightedButton == 1) 	{ this->highlightedButton = 0; }
			if (justPressed & RIGHT_BUTTON && this->highlightedButton == 0) { this->highlightedButton = 1; }
			if (justPressed & A_BUTTON) 		{ 
					
				switch (static_cast<Buttons>(this->highlightedButton)) {
					
					case Buttons::EndOfGame_Continue:
						changeView(machine, ViewState::StartHand); 
						break;

					case Buttons::EndOfGame_Quit:
						machine.changeState(GameStateType::TitleScreen); 
						break;

					default: break;
				
				}

			}

      break;

    case ViewState::Bust:
      
      this->buttonMode = ButtonDisplay::None;
          
      if (this->handInPlay == Hand::First) {

        if (arduboy.everyXFrames(FLASH_DELAY) && this->counter <= 4) {
Serial.println(this->counter);

          switch (this->counter) {

            case 0:
              this->player.firstHand.bust = true;
              Serial.println(F("Bust::playNextHand 11"));
              highlightLoss(this->handInPlay, this->player.firstHand.bet, (this->player.hasSecondHand() ? MessageNumber::BustFirstHand: MessageNumber::BustOnlyHand));
              break;

            case 4:
              Serial.println(F("Bust::playNextHand 12"));
              this->counter = 0;
              playNextHand(machine);
              break;

          }

          this->counter++;
        
        }

      }
      else if (this->handInPlay == Hand::Second) {
    
        this->player.secondHand.bust = true;
        // // this->winStatus = WinStatus::Lose;
        // // this->winStatusAmount = this->player.secondHand.bet;
        // // // drawPlayerScores();
        
        // // // highlightLoss(-player.secondHand.bet);
        // // playNextHand();
        // this->buttonMode = ButtonDisplay::None;
        // highlightLoss(this->handInPlay, this->player.secondHand.bet, MessageNumber::BustSecondHand);


        // // drawPlayerScores();
        
        // // highlightLoss(-player.firstHand.bet);

        // if (this->highlightEndOfGame.counter == 0) playNextHand(machine);

        if (arduboy.everyXFrames(FLASH_DELAY) && this->counter <= 4) {
Serial.println(this->counter);

          switch (this->counter) {

            case 0:
              this->player.secondHand.bust = true;
              Serial.println(F("Bust::playNextHand 21"));
              highlightLoss(this->handInPlay, this->player.secondHand.bet, MessageNumber::BustSecondHand);
              break;

            case 4:
              Serial.println(F("Bust::playNextHand 22"));
              playNextHand(machine);
              break;

          }

          this->counter++;
        
        }

      }

      break;

    default: break;

	}


}


void PlayGameState::playNextHand(StateMachine & machine) {
Serial.print(F("PlayNextHand: "));
Serial.println(player.firstHand.bust);

  player.split = false;
  
  if (this->handInPlay == Hand::First) {

    player.firstHand.stand = true;
    
    if (player.secondHand.cardCount > 0) {

      this->handInPlay = Hand::Second;
      changeView(machine, ViewState::PlayHand);
      
    }
    else {

      if (!player.firstHand.bust) {

        changeView(machine, ViewState::PlayDealerHand);

        // playDealerHand();
        
      }
      else {
       
        // showDealerCards();
        changeView(machine, ViewState::EndOfGame, 0, ButtonDisplay::EndOfGame);
Serial.println(player.firstHand.bust);        
      }
      
    }
    
  }
  else {

    if (!player.firstHand.bust || !player.secondHand.bust) {

      player.secondHand.stand = true;
      changeView(machine, ViewState::PlayDealerHand);
      // playDealerHand();
        
    }
    else {
       
      // showDealerCards();
      changeView(machine, ViewState::EndOfGame, 0, ButtonDisplay::EndOfGame);
      // drawButtons(); 
        
    }
    
  }
  
}


// ---------------------------------------------------------------------------------------------------------------------------
//  Render the state .. 
// ---------------------------------------------------------------------------------------------------------------------------
//
void PlayGameState::render(StateMachine & machine) {

	auto & arduboy = machine.getContext().arduboy;
	auto & ardBitmap = machine.getContext().ardBitmap;

	switch (this->viewState) {

		case ViewState::InitBet:

			font3x5.setCursor(0, 22);
			font3x5.print(F("Initial~bet? "));
			arduboy.fillRect(49, 22, 13, 7);
			font3x5.setTextColor(BLACK);
			render3DigitNumber(this->currentBetInit);
			font3x5.setTextColor(WHITE);
			break;

		case ViewState::OfferInsurance:
			drawPlayerHands(machine);
			drawDealerHand(machine, true);

			font3x5.setCursor(0, 22);
			font3x5.print(F("Insurance? "));
			arduboy.fillRect(43, 22, 13, 7);
			font3x5.setTextColor(BLACK);
			render3DigitNumber(this->insurance);
			font3x5.setTextColor(WHITE);

			break;

		case ViewState::InitDeal:
			drawPlayerHands(machine);
			drawDealerHand(machine, true);
			break;

		case ViewState::PeekOnTen:

			drawPlayerHands(machine);
			font3x5.setCursor(0, 22);

			switch (this->insuranceResult) {
				
				case InsuranceResult::Peeking:
					font3x5.print(F("Dealer~is~peeking~"));
					for (uint8_t x = 0; x < (this->counter / 8); x++) {
						font3x5.print(F("."));
					}
					drawDealerHand(machine, true);
					break;
				
				case InsuranceResult::BothHaveBlackjack:
					font3x5.print(F("Two~blackjacks!"));
					drawDealerHand(machine, false);
					break;
				
				case InsuranceResult::DealerHasBlackjack:
					font3x5.print(F("Dealer~has~Blackjack!"));
					drawDealerHand(machine, false);
					break;

				case InsuranceResult::DealerNoBlackjack:
					font3x5.print(F("Dealer~has~nothing!"));
					drawDealerHand(machine, true);
					break;

				case InsuranceResult::DealerHasBlackjackWithInsurance:
					font3x5.print(F("Insured Blackjack!"));
					drawDealerHand(machine, false);
					break;

			}
			break;

    case ViewState::SplitCards:
    case ViewState::PlayHand:
      drawPlayerHands(machine);
      drawDealerHand(machine, true);
      break;

    case ViewState::PlayDealerHand:
    case ViewState::CheckForWins:
      drawPlayerHands(machine);
      drawDealerHand(machine, false);
      break;

    case ViewState::Bust:
      drawPlayerHands(machine);
      drawDealerHand(machine, 
        (this->player.firstHand.bust && !this->player.hasSecondHand()) ||
        (this->player.firstHand.bust && this->player.hasSecondHand() && this->player.secondHand.bust)
      );
      break;

    case ViewState::EndOfGame:
      font3x5.setCursor(0, 22);
      drawPlayerHands(machine);
      drawDealerHand(machine, false);

      if (player.firstHand.bust) {
        font3x5.print(F("Bust!"));
      }

      break;

    default: break;

	}

  arduboy.fillRect(0, 52, WIDTH, HEIGHT, BLACK);
  ardBitmap.drawCompressed(0, 51, Images::Background, WHITE, ALIGN_NONE, MIRROR_NONE); 
  drawPlayerHands_Lines(machine);
	drawButtons(machine);
  drawStats(machine, this->highlightEndOfGame);

}
