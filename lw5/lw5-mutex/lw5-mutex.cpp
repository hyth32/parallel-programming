#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>

HANDLE FileLockingMutex;
HANDLE DepositMutex;
HANDLE WithdrawMutex;

int ReadFromFile() {
    WaitForSingleObject(FileLockingMutex, INFINITE);

    std::fstream myfile("balance.txt", std::ios_base::in);
    int result;
    myfile >> result;
    myfile.close();
    CloseHandle(FileLockingMutex);

    return result;
}

void WriteToFile(int data) {
    FileLockingMutex = CreateMutex(nullptr, FALSE, "FileLocking");
    WaitForSingleObject(FileLockingMutex, INFINITE);

    std::fstream myfile("balance.txt", std::ios_base::out);
    myfile << data << std::endl;
    myfile.close();

    ReleaseMutex(FileLockingMutex);
}

int GetBalance() {
    int balance = ReadFromFile();
    return balance;
}

void Deposit(int money) {
    WaitForSingleObject(DepositMutex, INFINITE);
    int balance = GetBalance();
    balance += money;

    WriteToFile(balance);

    printf("Balance after deposit: %d\n", balance);
    ReleaseMutex(DepositMutex);

}

void Withdraw(int money) {
    WaitForSingleObject(WithdrawMutex, INFINITE);

    if (GetBalance() < money) {
        printf("Cannot withdraw money, balance lower than %d\n", money);
        ReleaseMutex(WithdrawMutex);
        return;
    }

    Sleep(20);
    int balance = GetBalance();
    balance -= money;

    WriteToFile(balance);

    printf("Balance after withdraw: %d\n", balance);
    ReleaseMutex(WithdrawMutex);

}

DWORD WINAPI DoDeposit(LPVOID lpParameter) {
    Deposit((int) (LONG_PTR) lpParameter);
    ExitThread(0);
}

DWORD WINAPI DoWithdraw(LPVOID lpParameter) {
    Withdraw((int) (LONG_PTR) lpParameter);
    ExitThread(0);
}

int main() {
    auto *handles = new HANDLE[50];

    FileLockingMutex = CreateMutex(nullptr, FALSE, "FileLocking");
    DepositMutex = CreateMutex(nullptr, FALSE, "Deposit");
    WithdrawMutex = CreateMutex(nullptr, FALSE, "Withdraw");

    WriteToFile(0);

    SetProcessAffinityMask(GetCurrentProcess(), 1);
    for (int i = 0; i < 50; i++) {
        handles[i] = (i % 2 == 0)
                     ? CreateThread(nullptr, 0, &DoDeposit, (LPVOID) 230, CREATE_SUSPENDED, nullptr)
                     : CreateThread(nullptr, 0, &DoWithdraw, (LPVOID) 1000, CREATE_SUSPENDED, nullptr);
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(50, handles, TRUE, INFINITE);
    printf("Final Balance: %d\n", GetBalance());

    CloseHandle(FileLockingMutex);
    CloseHandle(DepositMutex);
    CloseHandle(WithdrawMutex);

    getchar();

    return 0;
}