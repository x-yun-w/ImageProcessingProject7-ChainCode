#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class point {
public:
	int r;
	int c;

	point() {}
	point(int a, int b) {
		r = a;
		c = b;
	}

	bool equal(point b) {
		return this->r == b.r && this->c == b.c;
	}
};

class chainCode {
public:
	int numR;
	int numC;
	int minV;
	int maxV;
	int label;
	int** ZFAry;
	int** reCAry;
	point corrdOffset[8] = { point(0,1), point(-1,1), point(-1,0), point(-1,-1), point(0,-1), point(1,-1), point(1,0), point(1,1) };
	int zeroTable[8] = { 6,0,0,2,2,4,4,6 };
	point startP;
	point currentP;
	point nextP;
	int lastQ;
	int chainDir;

	chainCode() {
	}

	void loadImage(ifstream& in) {
		for (int i = 1; i < numR + 1; i++) {
			for (int j = 1; j < numC + 1; j++) {
				in >> ZFAry[i][j];
			}
		}
	}

	void getChainCode(ofstream& chainCodeFile, ofstream& debug) {
		debug << "entering getChainCode method" << endl;
		chainCodeFile << numR << " " << numC << " " << minV << " " << maxV << endl;
		int i = 1;
		while (i < numR) {
			int j = 1;
			while (j < numC) {
				if (ZFAry[i][j] > 0)
				{
					label = ZFAry[i][j];
					chainCodeFile << label << " " << i << " " << j << endl;
					startP.r = i;
					startP.c = j;
					currentP.r = i;
					currentP.c = j;
					lastQ = 4;
					break;
				}
				j++;
			}
			if (ZFAry[i][j] > 0)
				break;
			i++;
		}

		while (!nextP.equal(startP))
		{
			lastQ = fmod(lastQ + 1, 8);

			chainDir = findNextP(debug);

			chainCodeFile << chainDir << " ";

			nextP.r = currentP.r + corrdOffset[chainDir].r;
			nextP.c = currentP.c + corrdOffset[chainDir].c;
			ZFAry[nextP.r][nextP.c] = label + 4;
			currentP = nextP;

			if (chainDir == 0)
				lastQ = zeroTable[7];
			else
				lastQ = zeroTable[chainDir - 1];

			debug << "lastQ = " << lastQ << " currentP.row = " << currentP.r << " currentP.col = " << currentP.c << " nextP.row = " << nextP.r << " nextP.col = " << nextP.c << endl;
		}
		debug << "leaving getChainCode" << endl;
	}

	int findNextP(ofstream& debug) {
		debug << "entering findNextP method" << endl;
		int index = lastQ;
		bool found = false;

		while (!found)
		{
			int ir = currentP.r + corrdOffset[index].r;
			int jc = currentP.c + corrdOffset[index].c;

			if (ZFAry[ir][jc] == label || ZFAry[ir][jc] == (label + 4)) {
				chainDir = index;
				found = true;
			}
			else
			{
				index = fmod(index + 1, 8);
			}
		}

		debug << "leaving findNextP method" << endl;
		debug << "chainDir = " << chainDir << endl;

		return chainDir;
	}

	void constructBoundary(ifstream& chainCodeFile, ofstream& debug) {
		debug << "Entering constructBoundary" << endl;
		chainCodeFile >> numR >> numC >> minV >> maxV;
		int i, j;
		chainCodeFile >> label >> i >> j;
		startP.r = i;
		startP.c = j;
		currentP.r = i;
		currentP.c = j;
		lastQ = 4;
		reCAry[startP.r][startP.c] = label;
		nextP.r = -1;
		nextP.c = 0;


		while (!nextP.equal(startP))
		{
			lastQ = fmod(lastQ + 1, 8);
			chainCodeFile >> chainDir;
			nextP.r = currentP.r + corrdOffset[chainDir].r;
			nextP.c = currentP.c + corrdOffset[chainDir].c;
			reCAry[nextP.r][nextP.c] = label;
			currentP = nextP;

			if (chainDir == 0)
				lastQ = zeroTable[7];
			else
				lastQ = zeroTable[chainDir - 1];

			debug << "lastQ = " << lastQ << " currentP.row = " << currentP.r << " currentP.col = " << currentP.c << " nextP.row = " << nextP.r << " nextP.col = " << nextP.c << endl;
		}

		debug << "Leaving constructBoundary" << endl;
	}

	void imgReformat(int** inAry, string d, ofstream& out) {
		out << numR << " " << numC << " " << minV << " " << maxV << endl;
		string s = to_string(maxV);

		int w = s.length();
		int r = 1;
		while (r < numR + 1) {
			int c = 1;
			while (c < numC + 1) {
				if (inAry[r][c] == 0) {
					out << d;
				}
				else {
					out << inAry[r][c];
				}
				s = to_string(inAry[r][c]);
				int ww = s.length();
				while (ww <= w) {
					out << " ";
					ww++;
				}
				c++;
			}
			out << endl;
			r++;
		}
	}


};

int main(int argc, char* argv[]) {
	ifstream in(argv[1]);
	ofstream out(argv[2]);
	ofstream debug(argv[3]);

	chainCode e;
	in >> e.numR >> e.numC >> e.minV >> e.maxV;

	e.ZFAry = new int* [e.numR + 2];
	e.reCAry = new int* [e.numR + 2];

	for (int j = 0; j < e.numR + 2; j++) {
		e.ZFAry[j] = new int[e.numC + 2];
		e.reCAry[j] = new int[e.numC + 2];
	}

	for (int i = 0; i < e.numR + 2; i++) {
		for (int j = 0; j < e.numC + 2; j++) {
			e.ZFAry[i][j] = 0;
			e.reCAry[i][j] = 0;
		}
	}

	string chainCodeFileName = (string)argv[1] + "_chainCode.txt";
	string boundaryFileName = (string)argv[1] + "_boundary.txt";
	ofstream chainCodeFile(chainCodeFileName);
	ofstream boundaryFile(boundaryFileName);

	e.loadImage(in);
	debug << "After loadImage; ZFAry as below" << endl;
	e.imgReformat(e.ZFAry, ".", out);

	e.getChainCode(chainCodeFile, debug);
	debug << "After getChainCode; ZFAry as below" << endl;
	e.imgReformat(e.ZFAry, ".", out);
	chainCodeFile.close();

	ifstream chainCodeFile1(chainCodeFileName);

	e.constructBoundary(chainCodeFile1, debug);
	e.imgReformat(e.reCAry, ".", out);
	e.imgReformat(e.reCAry, "0", boundaryFile);

	chainCodeFile1.close();
	in.close();
	out.close();
	debug.close();
	boundaryFile.close();
}