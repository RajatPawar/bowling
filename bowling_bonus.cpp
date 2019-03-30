#include <iostream>

using namespace std;

#define ALL_PINS     10

#define STRIKE_BONUS 10
#define SPARE_BONUS  5
#define STRIKE_LOOKAHEAD_BONUS 2
#define SPARE_LOOKAHEAD_BONUS 1

enum ROUND_STATES {
    ROUND_FINISHED = 1,
    ROUND_PROGRESS
};

enum CHANCE_ORDER {
    FIRST_CHANCE = 1,
    CHANCE_END
};

enum BONUS_TYPES {
    SPARE,
    STRIKE
};

class Player {
    
    public: 
    int curr_chance, curr_round, total_score, curr_round_score, state, last_round, given_last_chances, *chances_score, counter;
    
    Player() : curr_chance(0), curr_round(0), total_score(0), state(ROUND_PROGRESS), curr_round_score(0), given_last_chances(0), counter(0) {}
    ~Player() {}

    int isStrike();
    void init_chances_array(int);
    int isSpare();
    void print_score();
    int operator >(const Player &);
    int isRoundFinished();
    int isLastRound();
    int hasNextChance();
    int step_chance();
    void add_bonus(int);
    void complete_round(); // increment round
    int complete_chance(int); // increment chance
};

class Game {
    int rounds, players;
    Player *players_state;

    public:

    Game(int rounds, int players) : rounds(rounds), players(players) {
        players_state = new Player[players];
        for(int itr = 0; itr < players; itr++) 
            players_state[itr].init_chances_array(rounds);
    }
    ~Game() {}

    void start();
    void handle_player(int player, int round, int chance, int last_round);
    void swap(Player &, Player &);
    void sort();
    void print_scores();
    void calculate_bonuses();
};

int Player :: operator >(const Player &obj) {
    if(total_score > obj.total_score) return 1;
    return 0;
}

void Player :: init_chances_array(int rounds) {
    chances_score = new int[rounds * 2 + 2];
    for(int itr = 0; itr < rounds * 2 + 2; itr++)
        chances_score[itr] = 0;
}

int Player :: isStrike() {
    return curr_chance == FIRST_CHANCE && curr_round_score == ALL_PINS;
}

int Player :: isSpare() {
    return curr_chance != FIRST_CHANCE && curr_round_score == ALL_PINS;
}

void Player :: print_score() {
    cout<<"Total score: "<<total_score<<endl;
}

int Player :: isRoundFinished() {
    return state == ROUND_FINISHED;
}

int Player :: isLastRound() {
    return last_round == 1;
}

int Player :: hasNextChance() {
    return curr_chance < CHANCE_END;
}

int Player :: step_chance() {
    return (!isRoundFinished() && hasNextChance());
}

void Player :: add_bonus(int type) {
    switch(type) {
        case SPARE: {
            curr_round_score += SPARE_BONUS;
            break;
        }
        case STRIKE: {
            curr_round_score += STRIKE_BONUS;
            break;
        }
    }
}

void Player :: complete_round() {
    // Mark round state as finished
    state = ROUND_FINISHED;
    total_score += curr_round_score;
    
    // Reset vars
    curr_round_score = 0;
    curr_round++;
    
    // If it isn't the last round, reset chance variable too
    curr_chance = FIRST_CHANCE;
}

int Player :: complete_chance(int chance_score) {
    
    // Add to the current tally 
    chances_score[counter++] = chance_score;
    curr_round_score += chance_score;
    if(chance_score > 10) {
        cout<<"ERROR: Score cannot be more than 10"<<endl;
        exit(1);
    }
 
    if(isStrike()) {
        //add_bonus(STRIKE);
        cout<<"! Strike !"<<endl;
        if(!isLastRound()) complete_round();
        else {
            // Handle last round
            if(!given_last_chances) {
                given_last_chances = 1;
                return 1;
            }
        } 
    }
    else if(isSpare()) {
        //add_bonus(SPARE);
        cout<<"| Spare |"<<endl;
        if(!isLastRound()) complete_round();
        else {
            // Handle last round
            if(!given_last_chances) {
                given_last_chances = 1;
                return 1;
            }
        }
    }
   
    // Finish the round
    if(curr_chance == CHANCE_END) {
        complete_round();
    }
    
    return 0;
}

void Game :: handle_player(int curr_player, int curr_round, int curr_chance, int last_round = 0) {
    Player &player = players_state[curr_player];

    player.curr_round = curr_round;
    player.curr_chance = curr_chance;

    // Get the chance score input
    int chance_score;
    
    if(!last_round) 
        cout<<"Enter score for Player "<<curr_player + 1<<" for chance "<<curr_chance<<": ";
    else 
        cout<<"Enter score for Player "<<curr_player + 1<<" for bonus chance: ";
    
    cin>>chance_score;

    // Handle the chance score (strike or spare)
    int last_round_handling = player.complete_chance(chance_score);
    
    if(last_round_handling) {
        player.total_score += player.curr_round_score;
        player.curr_round_score = 0;
        handle_player(curr_player, curr_round, FIRST_CHANCE, 1);
    } 

    // If the player needs one more chance
    else if(player.step_chance())
        handle_player(curr_player, curr_round, ++player.curr_chance, last_round);
}


void Game :: start() {
    for(int curr_round = 0; curr_round < rounds; curr_round++) {
       cout<<"\n\n--------\nRound "<<curr_round + 1<<"\n-----------"<<endl;
       for(int curr_player = 0; curr_player < players; curr_player++) {
             
            Player &curr_ply = players_state[curr_player]; 
            
            if(curr_round == rounds - 1) {
                curr_ply.last_round = 1;
            }

            curr_ply.state = ROUND_PROGRESS;
            handle_player(curr_player, curr_round, FIRST_CHANCE);        
        }
       calculate_bonuses();
       print_scores();
    }
    //calculate_bonuses();
}

void Game :: calculate_bonuses() {

    for(int itr = 0; itr < players; itr++) {
        for(int score = 0; score < players_state[itr].counter; score++) {
            // Strike addon
            if(players_state[itr].chances_score[score] == 10) {
                for(int j = score + 1; j < score + STRIKE_LOOKAHEAD_BONUS + 1; j++) { 
                    cout<<"Player "<<itr<<" :Adding "<<players_state[itr].chances_score[j]<<endl;
                    if(j < players_state[itr].counter) players_state[itr].total_score += players_state[itr].chances_score[j];
                }
            }
            // Spare addon
            else if(players_state[itr].chances_score[score] + players_state[itr].chances_score[score + 1] == 10) {
                for(int j = score + 2; j < score + SPARE_LOOKAHEAD_BONUS + 2 && j < players_state[itr].counter; j++) {
                    cout<<"Player "<<itr<<" : SPARE: Adding "<<players_state[itr].chances_score[j]<<endl;
                    if(j < players_state[itr].counter) players_state[itr].total_score += players_state[itr].chances_score[j];
                }
            }
        }
    }
}

void Game :: swap(Player &a, Player &b) {
    Player temp = a;
    a = b;
    b = temp;
}

void Game :: sort() {
    for(int itr = 0; itr < players - 1; itr++) {
        for(int cmp_ob = 0; cmp_ob < players - itr - 1; cmp_ob++) {
            if(players_state[cmp_ob + 1] > players_state[cmp_ob]) {
                swap(players_state[cmp_ob + 1], players_state[cmp_ob]);
            }
        }
    }
}

void Game :: print_scores() {
    cout<<"\n\nRanking:\n------------\n\n";
    for(int itr = 0; itr < players; itr++) {
        cout<<"Player "<<itr + 1<<": ";
        players_state[itr].print_score();
    }
    cout<<endl<<endl;
}

int main() {
    int rounds, players;
    cout<<"\nEnter number of rounds & players: ";
    int input = scanf("%d %d", &rounds, &players);

    if(input != 2) {
        cout<<"ERROR: Input wrong";
        exit(1);
    }

    Game g(rounds, players);
    g.start();
    g.sort();
    //g.print_scores();

    return 0;
}
