#include "stdc++.h"
using namespace std;

typedef long double ld;

const ld QCST = log(10) / 400;
const ld PI = acos(-1); // lol ok
const ld CCST = 5.59016994653; // for 2.5 yrs, 50 RD to 175 RD, but daily lol
const int CURRYR = 21; // lol imagine TWOWing in 2021
const int ROUNDS = 100; // someone's finna break this eventually


class Player {
public:
	ld rm, rd, dfrd, prd; // RM, RD, de facto RD (for active fucks), and previous RD
	int rp, prp; // Rounds played, previous rounds played
	ld d2_inv, r_upd; // r_upd is the amount to adjust the player by after a day is completely processed
	map <int, ld> round_rm; // stores the entire round information for logging purposes; added by nerd
	Player(); // constructor, duh
	void time_step(); // go forward one day in time, flush out previous stats, update RD decay
	ld expected(Player& p); // expected probability that this player will beat Player p
	void add_match(ld w, ld s, Player& p, int round); // add single round-robin style match between another contestant in a round
	void update();
	ld g();	
};

Player::Player() {
	rm = 900;
	rd = 175;
	dfrd = 175; // i don't think this will break // update: it has not yet
	prd = 175;
	rp = 0;
	d2_inv = 0;
	r_upd = 0;
}

ld Player::g() {
	return 1 / sqrt(1 + 3 * QCST * QCST * (rd * rd) / (PI * PI)); // yeetus that fetus // future nerd here, wtf did past nerd mean
}

ld Player::expected(Player& p) {
	return 1 / (1 + pow(10, p.g() * (rm - p.rm) / -400));
}

void Player::time_step() {
	prd = dfrd; 
	rd = min(sqrt(rd * rd + CCST * CCST), ld(175));
	dfrd = rd; // ensures that the activity bonus is one day only
}
void Player::add_match(ld w, ld s, Player& p, int round) {
	ld e = expected(p);
	ld g = p.g();
	r_upd += w * g * (s - e); // okay i've gotta find out a way to store shit here or maybe make a new method for it, but the issue lies with global vs. local variables // update: nerd has fixed that
	if (round_rm.count(round) == 0) round_rm[round] = 0;
	round_rm[round] += w * g * (s - e);
	d2_inv += QCST * QCST * w * g * g * e * (1 - e);
}

void Player::update() {
	if (d2_inv > 0) {
		rm += QCST / (1 / (rd * rd) + d2_inv) * r_upd;
		rd = sqrt(1 / (1 / (rd * rd) + d2_inv));
		d2_inv = 0;
		r_upd = 0;
		if (rd < 50) {
			dfrd = rd;
			rd = (29*dfrd+50)/30;
		}
		else {
			dfrd = rd;
		}
	}
}

/*ld weight(int n) {
	if (n <= 2) return 1.;
	if (n <= 15) return 1./(.8769*n - .5848);
	return 1./(-.1037*n + 7.3273*sqrt(n) - 15.378);
}*/

ld weight(int n) {
	if (n < 5) return 1. / ld(n - 1);
	return 1. / (.0547 * n + 4 * sqrt(n) - 5.7663);
}

/*ld weight(int n) {
	return 1./ld(n-1);
}*/

int main() {
	cout.setf(ios::fixed);
	cout.precision(9);

	vector <string> folders;
	vector <vector<string>> rounds[CURRYR-15][12][31]; // rounds[yr][mo][da][j][k] refers to the kth ranked player in the jth round that happened on (yr-16)-(mo-1)-(da-1)
	vector <pair<string, int>> rnames[CURRYR-15][12][31]; // rnames[i] refers to the ith mth; any element contains the name of the TWOW and its round number
	map <string, vector<vector<ld>>> ratingmap; //a map of all players ratings over all the times (with the keys being player rnames)
	map <string, Player> playermap; // a map of username:current player data 
	map <string, map <string, vector<ld>>> mrounds; //mrounds[pname][rname] stores the round data for player pname and round rname - updated every month
	ifstream ifs; // input, reads in all data
	ofstream ofs; // spits out all numerical data for parsing later
	ofstream jrs; // round strengths json
	ofstream cms; // updated cleaner history practices
	ofstream dofs;
	string contestant;
	
	ofs.open("result.json");
	dofs.open("resultdaily.json");
	ifs.open("data/index.txt");
	jrs.open("rounds.json");
	cms.open("history.json");

	string s, t;
	while (getline(ifs, s)) { // reads in every season to look for
		folders.push_back(s);
	}
	ifs.close();

	ifs.open("data/alias.txt");
	map<string, string> alias;
	while (getline(ifs, s)) { // reads in every alias
		getline(ifs, t);
		alias[s] = t;
	}
	ifs.close();

	ifs.open("data/alts.txt");
	set<string> alts;
	while (getline(ifs, s)) { // reads in every blocked account
		alts.insert(s);
	}
	ifs.close();
	

	for (int i = 0; i < folders.size(); ++i) { // goes through entire list of season
		for (int j = 1; j <= ROUNDS; ++j) { // goes to maximum round number
			ifs.open("data/" + folders[i] + "/" + to_string(j) + ".txt");
			if (ifs.good()) {
				getline(ifs, s);
				int t = stoi(s); 	//representation of date yymmdd
				int dy = t%100;		//extracting year month and day
				int mth = (t/100)%100;
				int yr = ((t/100)/100)%100;
				
				cerr << folders[i] << " " << j << " " << t << endl;
				
				// add another round to rounds
				int k = rounds[yr-16][mth-1][dy-1].size(); 				//get the index of the new round
				rounds[yr-16][mth-1][dy-1].push_back(vector<string>()); 	//add an empty vector to store round leaderboard
				
				int pos = 0;
				while (getline(ifs, s)) {
					++pos;
					while (s[s.size() - 1] == ' ') s.pop_back();
					while (alias.count(s)) s = alias[s];
					if (alts.count(s) == 0) {rounds[yr-16][mth-1][dy-1][k].push_back(s);}
				}
				
				//add the new twow round to rnames as well
				rnames[yr-16][mth-1][dy-1].push_back(make_pair(folders[i] ,j)); 
				
			}
			ifs.close();
		}
	}

	ofs << "[";
	cms << "["; 
	jrs << "[";
	map<string, ld> pr;

	int mthdys[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	
	for (int yr = 16; yr < CURRYR; ++yr) {		//yr loops over all years (last two digits)
		for (int mth = 1; mth < 13; ++mth){		//mth loops over all months
			
			if (yr*100+mth != 1601) jrs << ","; //don't comma first month
			jrs << "{";
			if (yr*100+mth != 1601) ofs << ",";
			ofs << "{";
			if (yr*100+mth != 1601) cms << ",";
			cms << "{";
			
			
			int dycap = mthdys[mth-1];
			if (yr%4 == 0 and mth == 2) dycap++;  //I hate leap years
			
			
			bool firstrun1 = true;	//bool used for formatting: this one for rounds.json
			bool firstrun2 = true;	//bool used for formatting: this one for history.json
			bool firstrun3 = true;	//bool used for formatting: this one for result.json
			
			mrounds.clear();
			
			for (int dy = 1; dy < dycap+1; ++dy) {	//dy loops over all days
				
				if (mth < 10){cerr << "\rYEAR 20" << yr << " MONTH 0" << mth << " DAY " << dy;} 
				else {cerr << "\rYEAR 20" << yr << " MONTH " << mth << " DAY " << dy;}
				
				for (int j = 0; j < rounds[yr-16][mth-1][dy-1].size(); ++j) {			//j loops over the indexes of rounds that happened on yr-mth-dy
					int playernum = rounds[yr-16][mth-1][dy-1][j].size(); 				//the number of players who participated in the jth round of yr-mth-dy
					
					for (int p = 0; p < playernum; ++p) {	//p loops over the indexes of players in the jth round of yr-mth-dy
						string pname = rounds[yr-16][mth-1][dy-1][j][p]; //player name
						if (playermap.count(pname) == 0) playermap[pname] = Player(); // add player if they don't exist
						playermap[pname].rp++; //increase rp of player by one
					}
					
					//round strength stuff
					Player dummy = Player(); //create a dummy player
					dummy.rd = 50;
					dummy.rm = 1000;
					
					//increase and decrease the dummy's rm until it is even with the round's contestants (using a binary search kinda)
					ld step = 500;
					ld exp = 0;
					while (step > 0.01) { 
						exp = 0;
						for (int p1 = 0; p1 < playernum; ++p1) {
							exp += dummy.expected(playermap[rounds[yr-16][mth-1][dy-1][j][p1]]);
						}
						exp /= playernum;
						if (exp <= 0.5) dummy.rm += step;
						else dummy.rm += -step;
						step /= 2;
					}
					//record the strength of round to rounds.json
					if (!firstrun1){jrs << ",";} //don't add a comma if this is the first round in the month
					firstrun1 = false;
					
					jrs << "\"" << rnames[yr-16][mth-1][dy-1][j].first << " R" << rnames[yr-16][mth-1][dy-1][j].second << "\":["; 
					jrs << 5*(dummy.rm-2*dummy.rd) << ",";
					jrs << 10000*yr+100*mth+dy;
					jrs <<"]"; 
					
					//add the data of every matchup to each player
					for (int p1 = 0; p1 < playernum; ++p1) {			//{p1,p2} loops through every pair of indexes of players
						string pname1 = rounds[yr-16][mth-1][dy-1][j][p1]; 		//name of player one
						for (int p2 = p1 + 1; p2 < playernum; ++p2) {	
							string pname2 = rounds[yr-16][mth-1][dy-1][j][p2];
							playermap[pname1].add_match(weight(playernum), 1, playermap[pname2], j); // add all the victories of current player
							playermap[pname2].add_match(weight(playernum), 0, playermap[pname1], j); // and the defeats of the corresponding people
						}
					}
				}
				
				for (map<string, Player>::iterator p = playermap.begin(); p != playermap.end(); p++) { // p loops through the iterators of every player in playermap
					string pname = p->first;
					p->second.time_step(); 	//set the dfrd back to the capped rd first 
					
					//adding all of the rounds player p participated in to mrounds
					if (!p->second.round_rm.empty()) { // if player p participated in any rounds that day
					
						//add the player key if they don't exist
						if(mrounds.find(pname) == mrounds.end()){mrounds.insert(make_pair(pname,map <string, vector<ld>>()));}
						
						for (map<int, ld>::iterator q = p->second.round_rm.begin(); q != p->second.round_rm.end(); q++) { //q loops through the iterators of every round player p played in that day
							
							pair<string,int> rnamepr = rnames[yr-16][mth-1][dy-1][q->first]; //round name pair
							string rname = rnamepr.first + " R" + to_string(rnamepr.second);				//round name string
							
							//add the round key to mrounds[pname]
							mrounds[pname].insert(make_pair(rname,vector<ld>()));
							
							//add the score change to mrounds[pname][rname]
							float scorechange = 5 * QCST / (1 / (p->second.rd * p->second.rd) + p->second.d2_inv) * q->second;
							mrounds[pname][rname].push_back(scorechange);
							
							// open the round leaderboard and obtain the rank for said round
							ifs.open("data/" + rnamepr.first + "/" + to_string(rnamepr.second) + ".txt"); //open the file
							getline(ifs, s); //skip first line
							string loc; 	//variable to store each line
							int rank = 0;
							int pos = 0;
							while (getline(ifs, loc)) {
								++pos;
								while (loc[loc.size() - 1] == ' ') {loc.pop_back();} //remove extra spaces
								while (alias.count(loc)) {loc = alias[loc];} //do alias stuff
								if (pname == loc) rank = pos;  //if you find the player, save the read position as rank
								if (loc.empty()) pos--;	//if there's an empty line, pretend it doesn't exist
							}
							//when you exit this while loop, pos will be the contestant count.
							ifs.close();
							
							//add rank and contestant count to mrounds[pname][rname]
							mrounds[pname][rname].push_back(rank);
							mrounds[pname][rname].push_back(pos);
							
						}
					
						p->second.update();	//update player p's info
						
					}
					else {
						p->second.update(); //update player p's info (this is for when p has no rounds)
					}

					p->second.round_rm.clear(); //we're done using the round history so throw it away. h do be learning good habits tho :flushed:
					
					
					if (dy == dycap){		//if it is the end of the month
						if (firstrun3) {firstrun3 = false;} //don't add a comma for the first player of the month
						else {ofs << ",";}
						
						//add the player's data for the end of the month
						ofs << "\"" << pname << "\":[" << p->second.rm << "," << p->second.rd << "," << int(.5 + p->second.rp) << "," << p->second.dfrd << "]";
					}
					
					
					//adding stuff to ratingmap
					
					//if a player is not yet in ratingmap, add them
					if(ratingmap.find(pname) == ratingmap.end()){		
						ratingmap.insert(make_pair(pname,vector<vector<ld>>()));	//empty vector
						
						//add the first day that the player has a rating in ratingmap[pname][0][0]
						ratingmap[pname].push_back(vector<ld>());
						ratingmap[pname][0].push_back(10000*yr+100*mth+dy); //first vector is one that only has the first date
					}
					
					//add player data for the day to ratingmap
					vector<ld> rvec;
					rvec.push_back(p->second.dfrd);
					rvec.push_back(p->second.rm); //loading up the data into ratingmap
					rvec.push_back(int(.5 + p->second.rp));
					
					ratingmap[pname].push_back(rvec);
				}
			}
				
			//add stuff to history.json using data from mrounds
			for (map<string, map<string, vector<ld>>>::iterator p = mrounds.begin(); p != mrounds.end(); p++) { // p loops through the iterators of every player who played rounds
				string pname = p->first;	//player name
				
				if(p != mrounds.begin()){cms << ",";} //add a comma if not first player
				cms << "\"" << pname << "\":{";
				
				if(mrounds.find(pname) != mrounds.end()){	//if player pname participated in any rounds that month
					for (map<string, vector<ld>>::iterator q = mrounds[pname].begin(); q != mrounds[pname].end(); q++) { //q loops through the iterators of every round pname participated in the month
						
						if(q != mrounds[pname].begin()){cms << ",";} //add a comma if not first round
						
						string rname = q->first;	//round name
						cms << "\"" << rname << "\":[";
						
						cms << mrounds[pname][rname][0] << "," << mrounds[pname][rname][1] << "," << mrounds[pname][rname][2] << "]";
						
						
					}
				}
				cms << "}";
			}
			cerr << endl;
			
			ofs << "}";
			cms << "}";
			jrs << "}";
		}
	}
	
	
	
	int counter = 0;
	
	//adding stuff to "resultdaily.json"
	dofs << "{";
	
	for(map<string, vector<vector<ld>>>::iterator p = ratingmap.begin(); p != ratingmap.end(); p++){	//p loops through the iterators of every player
		string pname = p->first;	//the name of the player
		int stdate = int(p->second[0][0]+0.5);	//the first date they got a rating
		
		if (p != ratingmap.begin()){dofs << ",";}	//don't add a comma if it's the first player
		dofs << "\"" << pname << "\":[" << stdate;	//add the first date they got a rating
		
		counter++;
		cerr << "\rPlayers added: " << counter;
		
		for(int i = 1; i < p->second.size(); ++i){ //i loops over all of the indexes of ratings player p got
			dofs << ",[";
			
			dofs << p->second[i][0];	//add the rd
			
			//only add the rm and rp if the rp changed (this is to save memory because this file is very big)
			if (p->second[i][2] != p->second[i-1][2]){	
				dofs << ",";
				dofs << p->second[i][1];
				dofs << ",";
				dofs << p->second[i][2];
			} 
			
			dofs << "]";
		}
		dofs << "]";
	}
	dofs << "}";
	
	
	ofs << "]" << endl;
	cms << "]" << endl;
	jrs << "]" << endl;

	for (int i = 0; i < folders.size(); ++i) {
		set<string> ss;
		for (int j = 1; j <= ROUNDS; ++j) {
			ifs.open("data/" + folders[i] + "/" + to_string(j) + ".txt");
			if (ifs.good()) {
				getline(ifs, s);
				while (getline(ifs, s)) {
					while (s[s.size() - 1] == ' ') s.pop_back();
					while (alias.count(s)) s = alias[s];
					if (alts.count(s) == 0) ss.insert(s);
				}
			}
			ifs.close();
		}
		ld tpr = 0;
		for (string name : ss) tpr += pr[name];
	}
	dofs.close();
	jrs.close();
	ofs.close();
	cms.close();
}