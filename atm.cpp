#include <iostream>
#include <string>
#include <vector>
#include <ctime>
using namespace std;

// Forward declarations
class Card;
class Account;
class Transaction;
class Bank;

// Class Definitions
class Screen {
public:
    void displayMessage(string message) {
        cout << message << endl;
    }

    void displayOptions(vector<string> options) {
        cout << "Options: " << endl;
        for (int i = 0; i < options.size(); ++i) {
            cout << i + 1 << ". " << options[i] << endl;
        }
    }
};

class Keypad {
public:
    int getInput() {
        int input;
        cin >> input;
        return input;
    }

    float getAmount() {
        float amount;
        cin >> amount;
        return amount;
    }
};

class CashDispenser {
private:
    float cashAvailable;

public:
    CashDispenser(float cash) : cashAvailable(cash) {}

    bool dispenseCash(float amount) {
        if (cashAvailable >= amount) {
            cashAvailable -= amount;
            cout << "Dispensed Rs" << amount << endl;
            return true;
        } else {
            cout << "Insufficient cash in ATM." << endl;
            return false;
        }
    }

    float getCashAvailable() {
        return cashAvailable;
    }
};

class ReceiptPrinter {
public:
    void printReceipt(Transaction* transaction);
};

class Transaction {
private:
    static int nextTransactionID;
    int transactionID;
    string transactionType;
    float amount;
    time_t timestamp;
    string status;

public:
    Transaction(string type, float amt) : transactionType(type), amount(amt) {
        transactionID = nextTransactionID++;
        timestamp = time(0);
        status = "Pending";
    }

    void execute(Account* account);

    string getStatus() {
        return status;
    }

    float getAmount() {
        return amount;
    }

    string getTransactionType() {
        return transactionType;
    }

    friend class ReceiptPrinter;
};

int Transaction::nextTransactionID = 1;

class Account {
private:
    string accountNumber;
    float balance;

public:
    Account(string accNo, float bal) : accountNumber(accNo), balance(bal) {}

    bool withdraw(float amount) {
        if (balance >= amount) {
            balance -= amount;
            return true;
        } else {
            cout << "Insufficient funds in the account." << endl;
            return false;
        }
    }

    void deposit(float amount) {
        balance += amount;
    }

    float checkBalance() {
        return balance;
    }

    string getAccountNumber() {
        return accountNumber;
    }
};

class Card {
private:
    string cardNumber;
    string expiryDate;
    int pin; // Added pin attribute
    Account* account;

public:
    Card(string cardNo, string expiry, int pin, Account* acc)
        : cardNumber(cardNo), expiryDate(expiry), pin(pin), account(acc) {}

    bool validate() {
        // Add more validation logic if needed
        return true;
    }

    bool validatePIN(int inputPIN) {
        return pin == inputPIN;
    }

    Account* getAccount() {
        return account;
    }

    string getCardNumber() {
        return cardNumber;
    }
};

class Bank {
private:
    string name;
    vector<Card*> cards;
    vector<Account*> accounts;

public:
    Bank(string bankName) : name(bankName) {}

    void addCard(Card* card) {
        cards.push_back(card);
    }

    Card* getCardByNumber(string cardNumber) {
        for (Card* card : cards) {
            if (card->getCardNumber() == cardNumber) {
                return card;
            }
        }
        return nullptr;
    }

    void addAccount(Account* account) {
        accounts.push_back(account);
    }
};

class ATM {
private:
    string id;
    string location;
    CashDispenser cashDispenser;
    ReceiptPrinter receiptPrinter;
    Screen screen;
    Keypad keypad;
    Bank* bank;
    Card* currentCard = nullptr;  // Track the current active card
    bool cardInserted = true;
    const string adminPassword = "admin123"; // Admin password

public:
    ATM(string atmId, string loc, Bank* bankPtr)
        : id(atmId), location(loc), cashDispenser(5000), bank(bankPtr) {}

    void displayStartMenu() {
        screen.displayMessage("Welcome to the ATM!");
        screen.displayMessage("1. Admin Mode\n2. User Mode");
        int choice = keypad.getInput();

        if (choice == 1) {
            adminLogin();
        } else if (choice == 2) {
            userMode();
        } else {
            screen.displayMessage("Invalid choice. Please try again.");
            displayStartMenu();
        }
    }

    void adminLogin() {
        screen.displayMessage("Enter admin password:");
        string password;
        cin >> password;

        if (password == adminPassword) {
            adminMode();
        } else {
            screen.displayMessage("Incorrect password. Returning to main menu.");
            displayStartMenu();
        }
    }

    void adminMode() {
        screen.displayMessage("Admin Mode");
        vector<string> options = {"Add Card", "Exit Admin Mode"};
        screen.displayOptions(options);

        int choice = keypad.getInput();
        if (choice == 1) {
            addNewCard();
        } else if (choice == 2) {
            screen.displayMessage("Exiting Admin Mode...");
            displayStartMenu();
        } else {
            screen.displayMessage("Invalid choice.");
            adminMode();
        }
    }

    void addNewCard() {
        screen.displayMessage("Enter card number:");
        string cardNumber;
        cin >> cardNumber;

        screen.displayMessage("Enter expiry date (MM/YY):");
        string expiryDate;
        cin >> expiryDate;

        screen.displayMessage("Set a PIN for the card:");
        int pin = keypad.getInput();

        screen.displayMessage("Enter initial balance for the account in ₹:");
        float initialBalance = keypad.getAmount();

        // Create a new account and card, and add them to the bank
        Account* newAccount = new Account(cardNumber, initialBalance);
        bank->addAccount(newAccount);

        Card* newCard = new Card(cardNumber, expiryDate, pin, newAccount);
        bank->addCard(newCard);

        screen.displayMessage("New card added successfully!");

        // Return to admin menu
        adminMode();
    }

    void userMode() {
        screen.displayMessage("Insert your card (Enter card number):");
        string cardNumber;
        cin >> cardNumber;

        Card* card = bank->getCardByNumber(cardNumber);
        if (card != nullptr) {
            insertCard(card);
        } else {
            screen.displayMessage("Card not found. Returning to main menu.");
            displayStartMenu();
        }
    }

    void insertCard(Card* card) {
        if (card->validate()) {
            currentCard = card;  // Track the currently inserted card
            screen.displayMessage("Card Validated. Please enter your PIN:");
            int pin = keypad.getInput();

            if (card->validatePIN(pin)) {
                cardInserted = true; // Reset cardInserted flag
                while (cardInserted) {
                    displayMainMenu();
                }
            } else {
                screen.displayMessage("Invalid PIN. Ejecting card.");
                ejectCard();
            }
        } else {
            screen.displayMessage("Invalid Card. Ejecting card.");
            ejectCard();
        }
    }

    void displayMainMenu() {
        screen.displayMessage("Select transaction or eject card:");
        vector<string> options = {"Withdraw", "Deposit", "Check Balance", "Eject Card"};
        screen.displayOptions(options);

        int option = keypad.getInput();
        processTransaction(option);
    }

    void processTransaction(int transactionType) {
        if (currentCard == nullptr) {
            screen.displayMessage("No card inserted.");
            return;
        }

        Account* account = currentCard->getAccount();  // Only use the current card's account

        switch (transactionType) {
            case 1: { // Withdraw
                screen.displayMessage("Enter amount to withdraw in Rs:");
                float amount = keypad.getAmount();
                
                // Create a Transaction for the withdrawal attempt
                Transaction transaction("Withdrawal", amount);
                // Attempt to withdraw and dispense cash
                if (account->withdraw(amount) && cashDispenser.dispenseCash(amount)) {
                    transaction.execute(account);
                    receiptPrinter.printReceipt(&transaction);
                } else {
                    screen.displayMessage("Withdrawal failed. Transaction not completed.");
                }
                break;
            }
            case 2: { // Deposit
                screen.displayMessage("Enter amount to deposit in ₹:");
                float amount = keypad.getAmount();
                account->deposit(amount);
                screen.displayMessage("Deposit successful.");
                break;
            }
            case 3: { // Check Balance
                screen.displayMessage("Your balance is: ₹" + to_string(account->checkBalance()));
                break;
            }
            case 4: { // Eject Card
                ejectCard();
                break;
            }
            default:
                screen.displayMessage("Invalid option.");
                break;
        }
    }

    void ejectCard() {
        screen.displayMessage("Ejecting card...");
        currentCard = nullptr;  // Reset the current card
        cardInserted = false;
        displayStartMenu();  // Return to the main menu after card ejection
    }
};

void Transaction::execute(Account* account) {
    if (transactionType == "Withdrawal") {
        if (account->withdraw(amount)) {
            status = "Success";
            cout << "Transaction successful." << endl;
        } else {
            status = "Failure";
            cout << "Transaction failed." << endl;
        }
    }
}

void ReceiptPrinter::printReceipt(Transaction* transaction) {
    cout << "Receipt: " << endl;
    cout << "Transaction ID: " << transaction->transactionID << endl;
    cout << "Transaction Type: " << transaction->transactionType << endl;
    cout << "Amount: Rs" << transaction->amount << endl;
    cout << "Status: " << transaction->status << endl;
    cout << "Timestamp: " << ctime(&(transaction->timestamp)) << endl;
}

// Main function to simulate ATM usage
int main() {
    // Create a bank instance
    Bank bank("Sample Bank");

    // Create an ATM instance
    ATM atm("ATM1", "Main Street", &bank);

    // Display start menu (admin or user mode)
    atm.displayStartMenu();

    return 0;
}
