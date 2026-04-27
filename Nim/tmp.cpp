#include <iostream>
#include <string>
#include <vector>
using namespace std;

int numPiles;
vector<int> piles;

bool isServer{ true };
bool gameOver = false;
bool turn;
bool CheckPiles();
void CreatePile(string board);
void UpdateBoard(string move);
void EndGame(bool def = false);

string msg;
string blank;


string receive() {
    cout << "SEND AS OTHER USER: ";
    string result;

    cin >> result;
    return result;
}

void send(string msg) {
    cout << "RECEIVED AS OTHER USER: ";
    cout << msg << endl;
}

int main()
{
    if (isServer)
    {
        turn = false;
        bool validBoard = false;
        while (!validBoard)
        {
            getline(cin, msg);
            if (msg.size() - 1 != (msg.at(0) - '0') * 2)
            {
                cout << "Invalid Board\n";
            }
            else
            {
                CreatePile(msg);
                if (!CheckPiles())
                {
                    cout << "Invalid Board\n";
                }
                else
                {
                    validBoard = true;
                }
            }
        }
        send(msg);
    }
    else
    {
        CreatePile(receive());
        if (!CheckPiles())
        {
            EndGame(true);
        }
        turn = true;
        getline(cin, msg);
        UpdateBoard(msg);
        send(msg);
        turn = false;
    }

    cout << numPiles << endl;
    for (int i : piles)
    {
        cout << i << " ";
    }

    cout << endl;

    while (!gameOver)
    {
        msg = receive();
        UpdateBoard(msg);
        if (gameOver) break;
        turn = true;
        getline(cin, blank);
        getline(cin, msg);
        UpdateBoard(msg);
        send(msg);
        turn = false;
    }
    return 0;
}

bool CheckPiles()
{
    if (numPiles < 3 || numPiles > 9) return false;

    for (int i = 0; i < numPiles - 1; i++)
    {
        if (piles.at(i) < 1 || piles.at(i) > 20) return false;
    }

    return true;
}

void CreatePile(string board)
{
    numPiles = board[0] - '0';
    piles.resize(numPiles);

    int pilesIndex = 0;
    for (int i = 1; i <= numPiles * 2; i += 2)
    {
        if (board[i] - '0' == 1)
        {
            piles.at(pilesIndex) += (board[i + 1] - '0') + 10;
        }
        else if (board[i] - '0' == 2)
        {
            piles.at(pilesIndex) += (board[i + 1] - '0') + 20;
        }
        else if (board[i] - '0' > 2)
        {
            piles.at(pilesIndex) += 21;
        }
        else
        {
            piles.at(pilesIndex) += board[i + 1] - '0';
        }
        pilesIndex++;
    }
}

void UpdateBoard(string move)
{
    int selectedPile = move[0] - '0';

    while (selectedPile - 1 >= numPiles || selectedPile - 1 < 0)
    {
        cout << "Pile doesn't exist. Choose another pile.\n";
        getline(cin, msg);
        while (msg.size() != 1)
        {
            cout << "Please enter a single digit.\n";
            getline(cin, msg);
        }
        selectedPile = msg[0] - '0';
    }

    while (piles.at(selectedPile - 1) == 0)
    {
        cout << "Empty Pile. Choose another pile.\n";
        getline(cin, msg);
        while (msg.size() != 1)
        {
            cout << "Please enter a single digit.\n";
            getline(cin, msg);
        }
        selectedPile = msg[0] - '0';
    }

    int rocksRemove = 0;
    if (move[1] - '0' == 1)
    {
        rocksRemove += (move[2] - '0') + 10;
    }
    else if (move[1] - '0' == 2)
    {
        rocksRemove += (move[2] - '0') + 20;
    }
    else if (move[1] - '0' > 2)
    {
        rocksRemove += 21;
    }
    else
    {
        rocksRemove += move[2] - '0';
    }

    while (rocksRemove > piles.at(selectedPile - 1) || rocksRemove <= 0)
    {
        cout << "Unable to remove " << rocksRemove << " rocks. Choose another amount.\n";
        getline(cin, msg);
        while (msg.size() != 2)
        {
            cout << "Please enter two digits.\n";
            getline(cin, msg);
        }
        rocksRemove = 0;
        if (msg[0] - '0' == 1)
        {
            rocksRemove += (msg[1] - '0') + 10;
        }
        else if (msg[0] - '0' == 2)
        {
            rocksRemove += (msg[1] - '0') + 20;
        }
        else if (msg[0] - '0' > 2)
        {
            rocksRemove += 21;
        }
        else
        {
            rocksRemove += msg[1] - '0';
        }
    }

    piles.at(selectedPile - 1) -= rocksRemove;

    bool empty = true;
    for (int i : piles)
    {
        if (i != 0) empty = false;
    }

    for (int i : piles)
    {
        cout << i << " ";
    }

    cout << endl;

    if (empty)
    {
        EndGame();
    }
}

void EndGame(bool def)
{
    if (def) cout << "You win by default\n";

    if (turn) cout << "You win!\n";
    else cout << "You lose\n";
    gameOver = true;
}