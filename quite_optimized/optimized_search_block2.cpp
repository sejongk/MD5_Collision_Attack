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
		0x82000c61, 0xb24f1fc1, 0xf3fffff7, 0xffffffff, // 1~4
		0x7dffffff, 0x7fffffff, 0xfffefecf, 0xbfdff7ff, // 5~8
		0x3bca92fd, 0xf3ced2cd, 0xe14f72c5, 0xe10e7280, // 9~12
		0xe104e288, 0xe000a288, 0xe1018008, 0xe0020000, // 13~16
		0x80028008, 0xa0020000, 0x80020000, 0x80040000, // 17~20
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  block 1에서 구한 Q[61]~Q[64]를 이용해서 block 2의 initial state를 설정해 줘야함
	state[0] += Q[61]; // a, Q[-3] for second block
	state[1] += Q[64]; // b Q[0]
	state[2] += Q[63]; // c Q[-1]
	state[3] += Q[62]; // d Q[-2]
*/
Word state[4] = { 0x2b2d10f0, // Q[-3]
					0x21805223, //Q[0]
					0x196d1f1b, //Q[-1]
					0x15f67ec1 }; //Q[-2]
Word init_Q[3] = { 0x2b2d10f0, 0x15f67ec1, 0x196d1f1b };

Word Q[65] = { 0x21805223 , 0, };
Word M[16] = {};
/////////////////////////////////////////////////////////////////////////////////////////////////////////


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

void getQ(int t) {
	//t = 1~16 : round 0
	//17~32 : round 1
	//33~48 : round 2
	//49~64 : round 3
	int round = ((t - 1) / 16);

	Word P[4] = {
	(t - 4 < 0) ? init_Q[t - 1] : Q[t - 4],	// Q[t-4]
	(t - 3 < 0) ? init_Q[t] : Q[t - 3],		// Q[t-3]
	(t - 2 < 0) ? init_Q[t + 1] : Q[t - 2], // Q[t-2]
	Q[t - 1]								// Q[t-1]
	};
	Word newQ = P[0];
	
	Func(round, &newQ, P[3], P[2], P[1], M[W_idx[t - 1]], RC[t - 1], AC[t - 1]);
 	Q[t] = newQ;
}

void getW(int t) { // step t에서 사용되는 W(M)를 갱신
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

bool isFulfilled(int t, Word condition) {
	return (((Q[t] & mask[t]) ^ condition) == 0x00000000);

}
bool isFulfilled(int start, int end) {
	Word I = Q[46] & 0x80000000;
	Word J = Q[47] & 0x80000000;
	Word K = (~Q[46]) & 0x80000000;

	Word conditions[65]{
	0x00000000,
	(Q[0] & 0x80000000) + ((~Q[0]) & 0x00000020), (Q[1] & 0x80000000) + 0x30460400, (Q[2] & 0x80800002) + 0x103c32b0, (0x5157efd1 + (Q[3] & 0x08000000) + ((~Q[3]) & 0x80000000)),
	0x151900ab, 0x3347f06f, 0x79ea9e46, 0xa548136d,
	0x394002f1, 0xb2888208 + (Q[9] & 0x00044000), 0xe0444245 + (Q[10] & 0x00002000) + ((~Q[10]) & 0x00010000) , 0x810a1200,
	0x6104c008, 0xe000a288, (0xa0000008 + ((~Q[14]) & 0x00010000)), 0x80000000 + ((~Q[15]) & 0x00020000),
	(Q[16] & 0x00008008), 0x80020000 + (Q[17] & 0x20000000), 0x80020000, 0x80000000 + ((~Q[19]) & 0x00040000), // 17~20
	0x80000000 + (Q[20] & 0x00020000), 0x80000000, 0x00000000, 0x80000000, // 21~24
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
	return ((((T22 >> 17) & 1) == 1) && (((T34 >> 15) & 1) == 1));
}

void print_block2() { //little-endian 형식으로 block1 string을 출력 
	for (int i = 0; i <= 15; i++) {
		Word cur_M = M[i];
		for (int j = 0; j < 4; j++) {
			char c1 = ((cur_M >> 4) & 0xf);
			char c2 = (cur_M & 0xf);
			cur_M = (cur_M >> 8);

			(c1 < 10) ? (c1 += '0') : (c1 += ('a' - 10));
			(c2 < 10) ? (c2 += '0') : (c2 += ('a' - 10));

			cout << c1 << c2;
		}
	}
	cout << "\n";
}
typedef struct {
	Word origin_Q;
	int pos_cand[20];
	int pos_cnt = 0;

	int cur_idx = 0;
} Tunnel; // keep Tunnel state


Tunnel tunnel[4]; // tunnel[0] = tunnel Q9,9, tunnel[1] = tunnel Q9,10, tunnel[2] = tunnel Q10,10
int tunnel_Q[3] = { 9,9,10 };
int tunnel_str[3] = { 9,2,3 }; // Tunnel strength
int tunnel_pos[3][20] = { // Bit position candidates for Tunneling
	{ 1, 8, 10, 11, 16, 20, 21, 26, 30 },
	{ 13, 31 },
	{ 4, 5, 27 } 
};

void init_Tunnel(int t_idx) {
	Tunnel& t = tunnel[t_idx];

	t.origin_Q = Q[tunnel_Q[t_idx]];
	t.pos_cnt = 0;
	t.cur_idx = 0;
	for (int i = 0; i < tunnel_str[t_idx]; i++) {
		int pos = tunnel_pos[t_idx][i];
		switch (t_idx) {
		case 0:
			if ((((Q[10] >> pos) & 1) == 0) && (((Q[11] >> pos) & 1) == 1)) {
				t.pos_cand[t.pos_cnt++] = pos;
			}
			break;
		case 1:
			if ((((Q[10] >> pos) & 1) == 1) && (((Q[11] >> pos) & 1) == 1)) {
				t.pos_cand[t.pos_cnt++] = pos;
			}
			break;
		case 2:
			if ((((Q[11] >> pos) & 1) == 0)) {
				t.pos_cand[t.pos_cnt++] = pos;
			}
			break;
		}
	}

}
bool use_Tunnel(int t_idx) {
	Tunnel& t = tunnel[t_idx];
	if (t.cur_idx == (1 << t.pos_cnt))
		return false;

	Word newQ = t.origin_Q;
	for (int i = 0; i < t.pos_cnt; i++) {
		newQ ^= (((t.cur_idx >> i) & 1) << t.pos_cand[i]);
	}

	Q[tunnel_Q[t_idx]] = newQ;
	t.cur_idx++;
	return true;
}


void search_block2() {
	X = (Word)mix(clock() ^ time(NULL));
	
	while (true) {
		bool flag0 = false;

		while (!flag0) {
			Q[2] = (rng() & 0x4db0e03e) + 0x30460400 + (Q[0] & 0x80000000);
			Q[3] = (rng() & 0x0c000008) + 0x103c32b0 + (Q[2] & 0x80800002);
			Q[4] = (0x5157efd1 + (Q[3] & 0x08000000) + ((~Q[3]) & 0x80000000));
			Q[5] = (rng() & 0x82000000) + 0x151900ab;
			Q[6] = (rng() & 0x80000000) + 0x3347f06f;
			Q[7] = (rng() & 0x00010130) + 0x79ea9e46;
			Q[8] = (rng() & 0x40200800) + 0xa548136d;
			Q[9] = (rng() & 0xc4356d02) + 0x394002f1;
			Q[10] = (rng() & 0x0c312d32) + 0xb2888208 + (Q[9] & 0x00044000);
			Q[11] = (rng() & 0x1eb08d3a) + 0xe0444245 + (Q[10] & 0x00002000) + ((~Q[10]) & 0x00010000);
			Q[12] = (rng() & 0x1ef18d7f) + 0x810a1200;
			Q[13] = (rng() & 0x1efb1d77) + 0x6104c008;
			Q[14] = (rng() & 0x1fff5d77) + 0xe000a288;
			Q[15] = (rng() & 0x1efe7ff7) + 0xa0000008 + ((~Q[14]) & 0x00010000);
			Q[16] = (rng() & 0x1ffdffff) + 0x80000000 + ((~Q[15]) & 0x00020000);

			for (int i = 5; i <= 15; i++)
				getW(i);
			int cnt = 0;
			do {
				Q[1] = (rng() & 0x7dfff39e) + (Q[0] & 0x80000000) + ((~Q[0]) & 0x00000020);
				
				for (int i = 0; i <= 4; i++)
					getW(i);
				getQ(17);  getQ(18); getQ(19); getQ(20); getQ(21);
			} while (!isFulfilled(17, 21) && (cnt++) < 1000);

			if (isFulfilled(17, 21)) {
				flag0 = true;
			}

		}

		init_Tunnel(0); init_Tunnel(1); init_Tunnel(2);

		while (use_Tunnel(0)) { // Tunnel_Q9,9
			while (use_Tunnel(1)) { // Tunnel_Q9,10
				while (use_Tunnel(2)) { // Tunnel_Q10,10

					getW(8); getW(9); getW(10); getW(12); getW(13);

					for (int i = 22; i <= 64; i++) { // POV
						getQ(i);
					}

					if (isFulfilled(22, 64) && verify_T()) {
						for (int i = 0; i < 16; i++) {
							cout << "M[" << i << "] = " << M[i] << "\n";
						}
						for (int i = 0; i <= 64; i++) {
							cout << "Q[" << i << "] = " << Q[i] << "\n";
						}
						cout << "X => ";
						print_block2();

						M[4] -= ((Word)1 << 31);
						M[11] -= ((Word)1 << 15);
						M[14] -= ((Word)1 << 31);

						cout << "X' => ";
						print_block2();

						goto end;
					}

				}
			}
		}
	}
end:
	cout << "finish \n";

}


int main(void) {
	search_block2();
}

/*
string v_condition[65] = {
"................................",
"^.....0.............00...0!....0", "^.11..0..1..0110...0010000.....0", "^001..00^0111100001100101011.0^0", "!101^001010101111110111111010001",
".00101.1000110010000000010101011", ".0110011010001111111000001101111", "011110011110101.1001111.01..0110", "1.10010101.010000001.01101101101",
"..111.0101..0.0.0..0..1.111100.1", "1011..1010..1^0.1^.0..1.00..10.0", "111....0.1..010!.1^0..1.01...1.1", "100....1....101..001..1.0.......",
"011....1.....1..110...0.0...1...", "111.............1.1...1.1...1...", "101....0.......!0...........1...", "100...........!.................",
"0.............0.^...........^...", "1.^...........1.................", "1.............1.................", "1............!..................",
"1.............^.................", "1...............................", "0...............................", "1...............................",
"................................", "................................", "................................", "................................", // 25-28
"................................", "................................", "................................", "................................", // 29
"................................", "................................", "................................", "................................", // 33
"................................", "................................", "................................", "................................", // 37
"................................", "................................", "................................", "................................", // 41~44
"................................", "I...............................", "J...............................", "I...............................", // 45
"J...............................", "K...............................", "J...............................", "K...............................",
"J...............................", "K...............................", "J...............................", "K...............................",
"J...............................", "K...............................", "J...............................", "I...............................",
"J...............................", "I...............................", "J...............................", "................................",
};

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
bool v_isFulfilled(int t) {
	string cur_Q = wordToStr(Q[t]);
	string prev_Q = wordToStr(Q[t - 1]);
	char I = wordToStr(Q[46])[0], J = wordToStr(Q[47])[0], K = (char)('0' + !(bool)(I - '0'));

	for (int pos = 0; pos <= 31; pos++) {
		switch (v_condition[t][pos]) {
		case '.':	break;
		case '0':
			if (cur_Q[pos] != '0') return false; break;
		case '1':
			if (cur_Q[pos] != '1') return false; break;
		case '^':
			if (cur_Q[pos] != prev_Q[pos]) return false;  break;
		case '!':
			if (cur_Q[pos] != (!(bool)(prev_Q[pos] - '0') + '0')) return false; break;
		case 'I':
			if (cur_Q[pos] != I) return false; break;
		case 'J':
			if (cur_Q[pos] != J) return false; break;
		case 'K':
			if (cur_Q[pos] != K) return false; break;

		}
	}
	return true;
}



*/