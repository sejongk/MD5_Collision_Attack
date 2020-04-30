/*
This program demonstrates the method of tunneling and modified sufficient conditions according to
Marc Stevens. On collisions for md5. Master’s thesis, Eindhoven University of Technology, 6 2007

Random Number Generator(RNG) part used in this program came from https://github.com/s1fr0/md5-tunneling/blob/master/tunneling.c

References
[0] Ronald Rivest: The MD5 Message Digest Algorithm, RFC1321, April 1992,ftp://ftp.rfc-editor.org/in-notes/rfc1321.txt
[1] X. Wang and H. Yu: How to Break MD5 and Other Hash Functions., Eurocrypt'05, Springer-Verlag, LNCS, Vol. 3494, pp. 19-35. Springer, 2005.
[2] Vlastimil Klima: Tunnels in Hash Functions: MD5 Collisions Within a Minute, sent to IACR eprint, 18 March, 2006, http://eprint.iacr.org/2006/105.pdf
[3]Marc Stevens. On collisions for md5. Master’s thesis, Eindhoven University of Technology, 6 2007

*/

#include <iostream>
#include <string>
#include <ctime>
using namespace std;
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define ROTATE_RIGHT(x, n) (((x) >> (n)) | ((x) << (32-(n))))

typedef unsigned char Byte;
typedef unsigned int Word;
typedef unsigned long long Length;

Word RC[64] = {
	S11,S12,S13,S14,S11,S12,S13,S14,S11,S12,S13,S14,S11,S12,S13,S14, //0~15
	S21,S22,S23,S24,S21,S22,S23,S24,S21,S22,S23,S24,S21,S22,S23,S24, //16~31
	S31,S32,S33,S34,S31,S32,S33,S34,S31,S32,S33,S34,S31,S32,S33,S34, //32~47
	S41,S42,S43,S44,S41,S42,S43,S44,S41,S42,S43,S44,S41,S42,S43,S44  //48~63
};
Word AC[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
int W_idx[64] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, // 16~31
	5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, // 32~47
	0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9  // 48~63
};

Word mask[65] = {
		0x00000000,
		0x00000000, 0x00000000, 0x00080840, 0x80fffff0,
		0xfecffff5, 0xffffffff, 0xffffffff, 0xff9fafff,
		0xff1fbfff, 0xf09ff1c3, 0xf00ff1c3, 0xe30ff180,
		0xc30fe188, 0xa30fe188, 0xa3018008, 0xe0020000,
		0xc0028008, 0xa0020000, 0x80020000, 0x80040000,
		0x80020000, 0x80000000, 0x80000000, 0x80000000, // 21~24
		0x00000000, 0x00000000, 0x00000000, 0x00000000, // 25~28
		0x00000000, 0x00000000, 0x00000000, 0x00000000, // 29~32
		0x00000000, 0x00000000, 0x00000000, 0x00000000, // 33~36
		0x00000000, 0x00000000, 0x00000000, 0x00000000, // 37~40
		0x00000000, 0x00000000, 0x00000000, 0x00000000, // 41~44
		0x00000000, 0x80000000, 0x80000000, 0x80000000, // 45~48
		0x80000000, 0x80000000, 0x80000000, 0x80000000, // 49~52
		0x80000000, 0x80000000, 0x80000000, 0x80000000, // 53~56
		0x80000000, 0x80000000, 0x80000000, 0x80000000, // 57~60
		0x80000000, 0x80000000, 0x80000000, 0x00000000, // 61~64
};

Byte* input;
Byte output[16];

Word state[4] = { 0x67452301, //
					0xefcdab89, //Q[0]
					0x98badcfe, //Q[-1]
					0x10325476 }; //Q[-2]
Word init_Q[3] = { 0x67452301, 0x10325476, 0x98badcfe };

Word Q[65] = { 0xefcdab89, 0, };
Word M[16] = {};


void print_Q(Word Q) {
	for (int i = 31; i >= 0; i--) {
		bool outBit = (Q >> i) & 1;
		cout << outBit;
	}
	cout << "\n";
}
void Func(int round, Word* a, Word b, Word c, Word d, Word x, Word s, Word ac) {
	switch (round) {
	case 0: (*a) += F((b), (c), (d)) + (x)+(Word)(ac); break;
	case 1: (*a) += G((b), (c), (d)) + (x)+(Word)(ac); break;
	case 2: (*a) += H((b), (c), (d)) + (x)+(Word)(ac); break;
	case 3: (*a) += I((b), (c), (d)) + (x)+(Word)(ac); break;
	}
	(*a) = ROTATE_LEFT((*a), (s));
	(*a) += (b);
}


Word strToWord(string str) {
	Word ret = 0;
	Word tmp = 1;
	for (int i = 31; i >= 0; i--) {
		ret += (str[i] - '0') * tmp;
		tmp *= 2;
	}
	return ret;
}
string wordToStr(Word w) {
	string ret = "";
	for (int i = 31; i >= 0; i--) {
		ret += (char)('0' + ((w >> i) & 1));
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// https://github.com/s1fr0/md5-tunneling/blob/master/tunneling.c 에서 그대로 가져온 부분
//Robert Jenkins' 32 bit integer hash function
//Used to generate a good seed for rng()
Word mix(Word a) {
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}
//Random number generator. We will use an LCG pseudo random generator. Different options are possible
Word X;
Word rng(void) {
	X = (1103515245 * X + 12345) & 0xffffffff;
	return X;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	first block IHV
	0x67452301, //Q[-3]
	0x10325476,	//Q[-2]
	0x98badcfe, //Q[-1]
	0xefcdab89, //Q[0]	
*/
void getQ(int t) { // renew Q_t
	Word P[4] = {
	(t - 4 < 0) ? init_Q[t - 1] : Q[t - 4],	// Q[t-4]
	(t - 3 < 0) ? init_Q[t] : Q[t - 3],		// Q[t-3]
	(t - 2 < 0) ? init_Q[t + 1] : Q[t - 2], // Q[t-2]
	Q[t - 1]								// Q[t-1]
	};
	Word newQ = P[0];
	int round = ((t - 1) / 16);
	//t = 1~16 : round 0
	//17~32 : round 1
	//33~48 : round 2
	//49~64 : round 3

	Func(round, &newQ, P[3], P[2], P[1], M[W_idx[t - 1]], RC[t - 1], AC[t - 1]);

	Q[t] = newQ;
}

void getW(int t) { // renew W in step t(W_t)
	Word P[5] = {
	(t - 3 < 0) ? init_Q[t] : Q[t - 3],
	(t - 2 < 0) ? init_Q[t + 1] : Q[t - 2],
	(t - 1 < 0) ? init_Q[t + 2] : Q[t - 1],
	Q[t], Q[t + 1]
	};

	Word ret = ROTATE_RIGHT((P[4] - P[3]), RC[t]);

	switch (t / 16) {
	case 0: ret -= F(P[3], P[2], P[1]); break;
	case 1: ret -= G(P[3], P[2], P[1]); break;
	case 2: ret -= H(P[3], P[2], P[1]); break;
	case 3: ret -= I(P[3], P[2], P[1]); break;
	}
	ret -= (P[0] + AC[t]);

	M[W_idx[t]] = ret;
}

bool isFulfilled(int t, Word condition) { // check whether Q_t satisfy a given sufficient condition
	return (((Q[t] &  mask[t]) ^ condition) == 0x00000000);
	
}
bool isFulfilled(int start, int end) { // check Q_start ~ Q_end
	Word I = Q[46] & 0x80000000;
	Word J = Q[47] & 0x80000000;
	Word K = (~Q[46]) & 0x80000000;

	Word conditions[65]{
	0x00000000,
	0x00000000, 0x00000000, 0x00000000, (0x80080830 + (Q[3] & 0x0077f780)),
	0x88400025, 0x027fbc41 + (Q[5] & 0x0100000a), 0x03fef820, 0x01910540,
	0xfb102f3d + (Q[8] & 0x00001000), 0x701fd040, 0x2001c0c2, 0x00081100 + (Q[11] & 0x03000000),
	0x410fe008, 0x000be188, 0x21008000, 0x20000000 + ((~Q[15]) & 0x40020000),
	(Q[16] & 0x00008008) + ((~Q[16]) & 0x40000000), 0x00020000 + (Q[17] & 0x20000000), 0x00000000, ((~Q[19]) & 0x00040000),
	(Q[20] & 0x00020000), 0x00000000, 0x00000000, 0x80000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 25
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 29
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 33
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 37
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 41~44
	0x00000000, I, J, I, J, K, J, K,
	J, K, J, K, J, K, J, I,
	J, I, J, 0x00000000
	};

	for (int i = start; i <= end; i++)
		if (!isFulfilled(i, conditions[i])) {
			return false;
		}
	return true;
}
bool verify_T() { // T22[17]=0, T34[15]=0 for both block 1 and block 2
	//T22
	Word T22 = G(Q[22], Q[21], Q[20]) + Q[19] + AC[22] + M[W_idx[22]];
	//T34
	Word T34 = H(Q[34], Q[33], Q[32]) + Q[31] + AC[34] + M[W_idx[34]];
	return ((((T22 >> 17) & 1) == 0) && (((T34 >> 15) & 1) == 0));
}
bool verify_IHV() {
	Word state[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
	state[0] += Q[61]; // a, Q[-3] for second block
	state[1] += Q[64]; // b Q[0]
	state[2] += Q[63]; // c Q[-1]
	state[3] += Q[62]; // d Q[-2]

	string init[3] = { wordToStr(state[3]),wordToStr(state[2]),wordToStr(state[1]) };

	cout << init[0] << "\n";
	cout << init[1] << "\n";
	cout << init[2] << "\n";


	string init_condition[5][3] = {//Q[-2], Q[-1], Q[0]
		{"A.....0.........................", "A....01.........................", "A....00...................0....."}, // nr 0, IHV for Wang's second block IHV condition
		{"A.....0.........................", "A.....0........................1", "A.....0..................1.....1"}, // nr 1, IHV for new second block IHV condition nr 1(On collision for MD5 written by M.M.J. Stevens
		{"A.....0.........................", "A.....0........................0", "A.....0..................1.....0"}, // nr 2. 
		{"A.....0.........................", "A.....0........................1", "A.....0..................0.....1"}, // nr 3.
		{"A.....0.........................", "A.....0........................0", "A.....0..................0.....0"} // nr4.
	};

	char A = init[0][0];
	for (int nr = 0; nr < 5; nr++) {
		bool flag = true;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 32; j++) {
				switch (init_condition[nr][i][j]) {
				case '.': continue; break;
				case 'A':
					if (init[i][j] != A)
						flag = false;
					break;
				case '0':
					if (init[i][j] != '0')
						flag = false;
					break;
				case '1':
					if (init[i][j] != '1')
						flag = false;
					break;
				}
			}
		}
		if (flag == true) {
			cout << "satisfied IHV number: " << nr << "\n";
			return true;
		}
	}
	cout << "verifying IHV failed \n";
	return false;
}
void print_block1() { // Print block 1 by little-endian format
	for (int i = 0; i <= 15; i++) {
		Word cur_M = M[i];
		for (int j = 0; j < 4; j++) {
			char c1 = ((cur_M >> 4) & 0xf);
			char c2 = (cur_M & 0xf);
			cur_M = (cur_M >> 8);

			(c1 < 10) ? (c1 += '0') : (c1 += ('a' - 10));
			(c2 < 10) ? (c2 += '0') : (c2 += ('a' - 10));

			cout << c1 << c2 ;
		}
	}
	cout << "\n";
}
typedef struct {
	Word origin_Q;
	int pos_cand[20];
	int pos_cnt = 0;

	int cur_idx = 0;
} Tunnel; // keep Tunnel states


Tunnel tunnel[3]; // tunnel[0] = tunnel Q9,9, tunnel[1] = tunnel Q9,10, tunnel[2] = tunnel Q10,10
int tunnel_Q[3] = { 9,9,10 }; // tunnel에서 변경하고자 하는 Q
int tunnel_str[3] = { 3,1,11 }; // Tunnel strength
int tunnel_pos[3][20] = { // bit position 31,..., 0 used in each tunnel
	{ 21,22,23 },
	{ 14 },
	{ 2,3,4,5,9,10,11,24,25,26,27 }
};

void init_Tunnel(int t_idx) {
	Tunnel& t = tunnel[t_idx];
	
	t.origin_Q = Q[tunnel_Q[t_idx]];
	t.pos_cnt = 0;
	t.cur_idx = 0;
	for (int i = 0; i < tunnel_str[t_idx]; i++) {
		int pos = tunnel_pos[t_idx][i];
		switch (t_idx) { // check if ith bit in Q_t_idx satisfy a tunnel bit condition
		case 0: 
			if ((((Q[10] >> pos) & 1) == 0) && (((Q[11] >> pos) & 1) == 1)) { // conditions for Q9,9
				t.pos_cand[t.pos_cnt++] = pos; // if the condition is satisfied, it can be a candidate for this tunnel
			}
			break;
		case 1:
			if ((((Q[10] >> pos) & 1) == 1) && (((Q[11] >> pos) & 1) == 1)) { // conditions for Q9,10
				t.pos_cand[t.pos_cnt++] = pos;
			}
			break;
		case 2:
			if ((((Q[11] >> pos) & 1) == 0)) { // conditions for Q10,10
				t.pos_cand[t.pos_cnt++] = pos;
			}
			break;
		}
	}

}
bool use_Tunnel(int t_idx) {
	Tunnel& t = tunnel[t_idx];
	if (t.cur_idx == (1 << t.pos_cnt)) // if every possibe case is tried, stop the tunnel t_idx
		return false;

	Word newQ = t.origin_Q;
	for (int i = 0; i < t.pos_cnt; i++) {
		newQ ^= (((t.cur_idx >> i) & 1) << t.pos_cand[i]);
	}

	Q[tunnel_Q[t_idx]] = newQ; // change Q
	t.cur_idx++;
	return true;
}

void search_block1() {
	X = (Word)mix(clock() ^ time(NULL));

	while (true) {
		bool flag0 = false;
		
		while (!flag0) {
			Q[1] = rng();
			Q[3] = rng() & 0xfff7f7bf;
			Q[4] = (rng() & 0x7f00000f) + 0x80080830 + (Q[3] & 0x0077f780);
			Q[5] = (rng() & 0x0130000a) + 0x88400025;
			Q[6] = 0x027fbc41 + (Q[5] & 0x0100000a);
			Q[7] = 0x03fef820;
			Q[8] = (rng() & 0x00605000) + 0x01910540;
			Q[9] = (rng() & 0x00e04000) + 0xfb102f3d + (Q[8] & 0x00001000);
			Q[10] = (rng() & 0x0f600e3c) + 0x701fd040;
			Q[11] = (rng() & 0x0ff00e3c) + 0x2001c0c2;
			Q[12] = (rng() & 0x1cf00e7f) + 0x00081100 + (Q[11] & 0x03000000);
			Q[13] = (rng() & 0x3cf01e77) + 0x410fe008;
			Q[14] = (rng() & 0x5cf01e77) + 0x000be188;
			Q[15] = (rng() & 0x5cfe7ff7) + 0x21008000;
			Q[16] = (rng() & 0x1ffdffff) + 0x20000000 + ((~Q[15]) & 0x40020000);
		
			getW(0);
			for (int i = 6; i <= 15; i++)
				getW(i);

			int cnt = 0;
			do {
				Q[17] = (rng() & 0x3ffd7ff7) + (Q[16] & 0x00008008) + ((~Q[16]) & 0x40000000);
				getW(16); // renew M[1]
				getQ(2);

				getW(2); getW(3); getW(4); getW(5);
				getQ(18); getQ(19); getQ(20); getQ(21);
			} while (!isFulfilled(18, 21) && (cnt++) < 1000);

			if (isFulfilled(18, 21)) {
				flag0 = true;
			}

		}

		init_Tunnel(0); init_Tunnel(1); init_Tunnel(2);

		while (use_Tunnel(0)) { // Tunnel_Q9,9
			while (use_Tunnel(1)) { // Tunnel_Q9,10
				while (use_Tunnel(2)) { // Tunnel_Q10,10

					getW(8); getW(9); getW(10); getW(12); getW(13); // m8,9,10,12,13 갱신
			
					for (int i = 22; i <= 64; i++) { // POV
						getQ(i);
					}
				
					if (isFulfilled(22,64) && verify_T() && verify_IHV()) {
						cout << "block 1 found!\n";
						for (int i = 0; i < 16; i++) {
							cout << "M[" << i << "] = " << M[i] << "\n";
						}
						for (int i = 0; i <= 64; i++) {
							cout << "Q[" << i << "] = " << Q[i] << "\n";
						}
						cout << "X => ";
						print_block1();

						M[4] += ((Word)1 << 31);
						M[11] += ((Word)1 << 15);
						M[14] += ((Word)1 << 31);

						cout << "X' => ";
						print_block1();

						goto end;
					}

				}
			}
		}
	}
end:
	cout << "finish \n";

}
void show_Q_diff() {
	Word tmp_Q[65] = { 0, };
	/*
	M[0] = 492416569;
	M[1] = 3453619469;
	M[2] = 276887159;
	M[3] = 490809300;
	M[4] = 2192778538;
	M[5] = 2054400027;
	M[6] = 363998961;
	M[7] = 3866060590;
	M[8] = 86240577;
	M[9] = 3786667003;
	M[10] = 2107944331;
	M[11] = 306354987;
	M[12] = 1547819276;
	M[13] = 549319407;
	M[14] = 846701524;
	M[15] = 4164812451;
	*/
	tmp_Q[0] = Q[0];
	for (int i = 1; i <= 64; i++) {
		getQ(i);
		tmp_Q[i] = Q[i];
	}
	// get M'[0]~M'[15]
	M[4] += ((Word)1 << 31);
	M[11] += ((Word)1 << 15);
	M[14] += ((Word)1 << 31);

	state[0] = 0x67452301; // Q[-3]
	state[1] = 0xefcdab89; // Q[0]
	state[2] = 0x98badcfe; // Q[-1]
	state[3] = 0x10325476; // Q[-2]

	init_Q[0] = 0x67452301;
	init_Q[1] = 0x10325476;
	init_Q[2] = 0x98badcfe;

	Q[0] = 0xefcdab89;

	for (int i = 1; i <= 64; i++) {
		getQ(i);
	}
	for (int i = 0; i <= 64; i++) {
		cout << "Q diff " << i << " => ";
		print_Q((Word)(Q[i] - tmp_Q[i]));
	}

}

int main(void) {
	search_block1();
	show_Q_diff();
}