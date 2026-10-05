#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// ------------------------------------------------
// SYSTEM CONSTANTS
// ------------------------------------------------
#define MAX_MEMBERS 100
#define MAX_LOANS   100
#define MAX_TXNS    300

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// ------------------------------------------------
// DATA STRUCTURES
// ------------------------------------------------
typedef struct {
    int  id;
    char name[50];
    char nid[20];
    char password[20];
    char role[20];       // Admin, Officer, Member
    char phone[15];
    float savingsBalance;
    int  activeLoanId;   // -1 if none
    int  isActive;
} Member;

typedef struct {
    int   loanId;
    int   memberId;
    float principal;
    float interestRate;    // annual, simple interest
    float totalPayable;
    float amountRepaid;
    char  purpose[100];
    char  status[20];      // Pending, Disbursed, Closed, Rejected
    time_t appliedDate;
    time_t approvedDate;
} Loan;

typedef struct {
    int   memberId;
    char  type[15];        // Deposit, Withdraw, Disbursement, Repayment
    float amount;
    char  date[30];
} Transaction;

Member members[MAX_MEMBERS];
Loan loans[MAX_LOANS];
Transaction txns[MAX_TXNS];
int memberCount = 0, loanCount = 0, txnCount = 0;
int currentMemberId = -1;
char currentRole[20] = "";

const char* MEMBER_FILE = "members.dat";
const char* LOAN_FILE   = "loans.dat";
const char* TXN_FILE    = "transactions.dat";

// ------------------------------------------------
// PROTOTYPES
// ------------------------------------------------
void clearConsole();
void waitForKeyPress();
void getCurrentDateTime(char* buffer);
void saveAllData();
void loadAllData();
void initializeSystem();
void showWelcomeBanner();
void showPageHeader(const char* title);

int validateNID(const char* nid);
int findMemberByNID(const char* nid);
int findLoanById(int id);

void registerNewMember();
void memberLogin();
void showProfile();
void depositFunds();
void withdrawFunds();
void applyForLoan();
int  checkLoanEligibility(int memberIndex, float requestedAmount);
void reviewPendingLoans();
void approveOrRejectLoan();
void recordRepayment();
void viewMyLoanStatus();
void recordTransaction(int memberId, const char* type, float amount);
void generateSystemReport();
void addNewOfficer();
void listAllMembers();

void showAdminMenu();
void showOfficerMenu();
void showMemberMenu();
void showMainMenu();

// ------------------------------------------------
// UTILITIES
// ------------------------------------------------
void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitForKeyPress() {
    printf("\n" YELLOW "Press ENTER to continue..." RESET);
    while (getchar() != '\n') {}
    getchar();
}

void getCurrentDateTime(char* buffer) {
    time_t now;
    struct tm* lt;
    time(&now);
    lt = localtime(&now);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", lt);
}

// ------------------------------------------------
// VALIDATION
// ------------------------------------------------
int validateNID(const char* nid) {
    int len = strlen(nid);
    if (len != 10 && len != 17) return 0;
    for (int i = 0; i < len; i++) {
        if (!isdigit((unsigned char)nid[i])) return 0;
    }
    return 1;
}

int findMemberByNID(const char* nid) {
    for (int i = 0; i < memberCount; i++) {
        if (strcmp(members[i].nid, nid) == 0) return i;
    }
    return -1;
}

int findLoanById(int id) {
    for (int i = 0; i < loanCount; i++) {
        if (loans[i].loanId == id) return i;
    }
    return -1;
}

// ------------------------------------------------
// PERSISTENCE
// ------------------------------------------------
void saveAllData() {
    FILE* fp;

    fp = fopen(MEMBER_FILE, "wb");
    if (fp) {
        fwrite(&memberCount, sizeof(int), 1, fp);
        fwrite(members, sizeof(Member), memberCount, fp);
        fclose(fp);
    }

    fp = fopen(LOAN_FILE, "wb");
    if (fp) {
        fwrite(&loanCount, sizeof(int), 1, fp);
        fwrite(loans, sizeof(Loan), loanCount, fp);
        fclose(fp);
    }

    fp = fopen(TXN_FILE, "wb");
    if (fp) {
        fwrite(&txnCount, sizeof(int), 1, fp);
        fwrite(txns, sizeof(Transaction), txnCount, fp);
        fclose(fp);
    }
}

void loadAllData() {
    FILE* fp;

    fp = fopen(MEMBER_FILE, "rb");
    if (fp) {
        fread(&memberCount, sizeof(int), 1, fp);
        fread(members, sizeof(Member), memberCount, fp);
        fclose(fp);
    }

    fp = fopen(LOAN_FILE, "rb");
    if (fp) {
        fread(&loanCount, sizeof(int), 1, fp);
        fread(loans, sizeof(Loan), loanCount, fp);
        fclose(fp);
    }

    fp = fopen(TXN_FILE, "rb");
    if (fp) {
        fread(&txnCount, sizeof(int), 1, fp);
        fread(txns, sizeof(Transaction), txnCount, fp);
        fclose(fp);
    }
}

// ------------------------------------------------
// DEFAULT ACCOUNTS
// ------------------------------------------------
void initializeSystem() {
    if (memberCount == 0) {
        Member admin = {1, "System Admin", "0000000001", "admin123", "Admin", "01700000000", 0, -1, 1};
        Member officer = {2, "Loan Officer One", "0000000002", "officer123", "Officer", "01711111111", 0, -1, 1};
        members[memberCount++] = admin;
        members[memberCount++] = officer;
        saveAllData();
    }
}

// ------------------------------------------------
// BANNER / HEADERS
// ------------------------------------------------
void showWelcomeBanner() {
    clearConsole();
    printf(CYAN "============================================================\n" RESET);
    printf(BOLD "        MICROFINANCE & COOPERATIVE MANAGEMENT SYSTEM\n" RESET);
    printf(CYAN "============================================================\n" RESET);
    printf(YELLOW " Course: Software Development Capstone Project\n" RESET);
    printf(YELLOW " Submitted By: [Your Name] | ID: [Your ID]\n" RESET);
    printf(YELLOW " Submitted To: [Teacher's Name]\n" RESET);
    printf(CYAN "------------------------------------------------------------\n" RESET);
    printf(" Default Logins:\n");
    printf("   Admin   : NID 0000000001 / admin123\n");
    printf("   Officer : NID 0000000002 / officer123\n");
    printf(CYAN "============================================================\n" RESET);
    waitForKeyPress();
}

void showPageHeader(const char* title) {
    clearConsole();
    printf(CYAN "============================================================\n" RESET);
    printf(BOLD " %s\n" RESET, title);
    printf(CYAN "============================================================\n\n" RESET);
}

// ------------------------------------------------
// REGISTRATION & LOGIN
// ------------------------------------------------
void registerNewMember() {
    showPageHeader("MEMBER REGISTRATION");

    if (memberCount >= MAX_MEMBERS) {
        printf(RED "Member limit reached!\n" RESET);
        waitForKeyPress();
        return;
    }

    Member m;
    m.id = memberCount + 1;
    m.isActive = 1;
    m.savingsBalance = 0;
    m.activeLoanId = -1;
    strcpy(m.role, "Member");

    printf("Enter Full Name: ");
    scanf(" %[^\n]", m.name);

    do {
        printf("Enter National ID (10 or 17 digits): ");
        scanf(" %[^\n]", m.nid);
        if (!validateNID(m.nid)) {
            printf(RED "Invalid NID format!\n" RESET);
        } else if (findMemberByNID(m.nid) != -1) {
            printf(RED "This NID is already registered!\n" RESET);
        }
    } while (!validateNID(m.nid) || findMemberByNID(m.nid) != -1);

    printf("Enter Password: ");
    scanf(" %[^\n]", m.password);
    printf("Enter Phone Number: ");
    scanf(" %[^\n]", m.phone);

    members[memberCount++] = m;
    saveAllData();

    printf(GREEN "\nRegistration successful! Your Member ID is: %d\n" RESET, m.id);
    waitForKeyPress();
}

void memberLogin() {
    showPageHeader("LOGIN");

    char nid[20], password[20];
    printf("National ID: ");
    scanf(" %[^\n]", nid);
    printf("Password: ");
    scanf(" %[^\n]", password);

    for (int i = 0; i < memberCount; i++) {
        if (strcmp(members[i].nid, nid) == 0 &&
            strcmp(members[i].password, password) == 0 &&
            members[i].isActive) {
            currentMemberId = i;
            strcpy(currentRole, members[i].role);
            printf(GREEN "\nWelcome, %s! Logged in as %s.\n" RESET, members[i].name, members[i].role);
            waitForKeyPress();
            return;
        }
    }
    printf(RED "\nInvalid NID or password!\n" RESET);
    waitForKeyPress();
}

// ------------------------------------------------
// PROFILE
// ------------------------------------------------
void showProfile() {
    if (currentMemberId == -1) { printf(RED "Please login first!\n" RESET); waitForKeyPress(); return; }
    showPageHeader("MY PROFILE");
    Member* m = &members[currentMemberId];
    printf("ID               : %d\n", m->id);
    printf("Name             : %s\n", m->name);
    printf("NID              : %s\n", m->nid);
    printf("Role             : %s\n", m->role);
    printf("Phone            : %s\n", m->phone);
    printf("Savings Balance  : %.2f\n", m->savingsBalance);
    waitForKeyPress();
}

// ------------------------------------------------
// SAVINGS OPERATIONS
// ------------------------------------------------
void recordTransaction(int memberId, const char* type, float amount) {
    if (txnCount >= MAX_TXNS) return;
    Transaction t;
    t.memberId = memberId;
    strcpy(t.type, type);
    t.amount = amount;
    getCurrentDateTime(t.date);
    txns[txnCount++] = t;
}

void depositFunds() {
    if (currentMemberId == -1) { printf(RED "Please login first!\n" RESET); waitForKeyPress(); return; }
    showPageHeader("DEPOSIT FUNDS");

    float amount;
    printf("Enter amount to deposit: ");
    scanf("%f", &amount);

    if (amount <= 0) {
        printf(RED "Invalid amount!\n" RESET);
        waitForKeyPress();
        return;
    }

    members[currentMemberId].savingsBalance += amount;
    recordTransaction(members[currentMemberId].id, "Deposit", amount);
    saveAllData();

    printf(GREEN "Deposit successful! New balance: %.2f\n" RESET, members[currentMemberId].savingsBalance);
    waitForKeyPress();
}

void withdrawFunds() {
    if (currentMemberId == -1) { printf(RED "Please login first!\n" RESET); waitForKeyPress(); return; }
    showPageHeader("WITHDRAW FUNDS");

    float amount;
    printf("Enter amount to withdraw: ");
    scanf("%f", &amount);

    if (amount <= 0 || amount > members[currentMemberId].savingsBalance) {
        printf(RED "Invalid amount or insufficient balance!\n" RESET);
        waitForKeyPress();
        return;
    }

    members[currentMemberId].savingsBalance -= amount;
    recordTransaction(members[currentMemberId].id, "Withdraw", amount);
    saveAllData();

    printf(GREEN "Withdrawal successful! New balance: %.2f\n" RESET, members[currentMemberId].savingsBalance);
    waitForKeyPress();
}

// ------------------------------------------------
// LOAN OPERATIONS
// ------------------------------------------------
int checkLoanEligibility(int memberIndex, float requestedAmount) {
    return (members[memberIndex].savingsBalance * 5 >= requestedAmount)
        && (members[memberIndex].activeLoanId == -1);
}

void applyForLoan() {
    if (currentMemberId == -1) { printf(RED "Please login first!\n" RESET); waitForKeyPress(); return; }
    showPageHeader("APPLY FOR LOAN");

    if (loanCount >= MAX_LOANS) {
        printf(RED "Loan capacity reached!\n" RESET);
        waitForKeyPress();
        return;
    }

    float amount;
    printf("Enter requested loan amount: ");
    scanf("%f", &amount);

    if (!checkLoanEligibility(currentMemberId, amount)) {
        printf(RED "Not eligible! Loan must be <= 5x your savings balance,\n");
        printf("and you must not already have an active loan.\n" RESET);
        waitForKeyPress();
        return;
    }

    Loan l;
    l.loanId = loanCount + 1;
    l.memberId = members[currentMemberId].id;
    l.principal = amount;
    l.interestRate = 0;
    l.totalPayable = 0;
    l.amountRepaid = 0;
    strcpy(l.status, "Pending");
    time(&l.appliedDate);

    printf("Enter purpose of loan: ");
    scanf(" %[^\n]", l.purpose);

    loans[loanCount++] = l;
    saveAllData();

    printf(GREEN "\nLoan application #%d submitted! Awaiting officer review.\n" RESET, l.loanId);
    waitForKeyPress();
}

void reviewPendingLoans() {
    showPageHeader("PENDING LOAN APPLICATIONS");

    printf("%-5s %-10s %-10s %-30s\n", "ID", "MemberID", "Amount", "Purpose");
    printf("------------------------------------------------------------\n");
    int found = 0;
    for (int i = 0; i < loanCount; i++) {
        if (strcmp(loans[i].status, "Pending") == 0) {
            printf("%-5d %-10d %-10.2f %-30s\n",
                   loans[i].loanId, loans[i].memberId, loans[i].principal, loans[i].purpose);
            found++;
        }
    }
    if (!found) printf("No pending loan applications.\n");
    waitForKeyPress();
}

void approveOrRejectLoan() {
    if (strcmp(currentRole, "Admin") != 0 && strcmp(currentRole, "Officer") != 0) {
        printf(RED "Restricted to Admin / Officer only!\n" RESET);
        waitForKeyPress();
        return;
    }
    showPageHeader("APPROVE / REJECT LOAN");

    int id;
    printf("Enter Loan ID: ");
    scanf("%d", &id);

    int idx = findLoanById(id);
    if (idx == -1 || strcmp(loans[idx].status, "Pending") != 0) {
        printf(RED "Invalid loan ID or not pending!\n" RESET);
        waitForKeyPress();
        return;
    }

    char choice;
    printf("Approve this loan? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        float rate;
        printf("Enter annual interest rate (%%): ");
        scanf("%f", &rate);

        loans[idx].interestRate = rate;
        loans[idx].totalPayable = loans[idx].principal * (1 + rate / 100.0f);
        strcpy(loans[idx].status, "Disbursed");
        time(&loans[idx].approvedDate);

        for (int i = 0; i < memberCount; i++) {
            if (members[i].id == loans[idx].memberId) {
                members[i].activeLoanId = loans[idx].loanId;
                recordTransaction(members[i].id, "Disbursement", loans[idx].principal);
                break;
            }
        }
        printf(GREEN "Loan approved and disbursed!\n" RESET);
    } else {
        strcpy(loans[idx].status, "Rejected");
        printf(YELLOW "Loan rejected.\n" RESET);
    }

    saveAllData();
    waitForKeyPress();
}

void recordRepayment() {
    if (strcmp(currentRole, "Admin") != 0 && strcmp(currentRole, "Officer") != 0) {
        printf(RED "Restricted to Admin / Officer only!\n" RESET);
        waitForKeyPress();
        return;
    }
    showPageHeader("RECORD REPAYMENT");

    int id;
    float amount;
    printf("Enter Loan ID: ");
    scanf("%d", &id);

    int idx = findLoanById(id);
    if (idx == -1 || strcmp(loans[idx].status, "Disbursed") != 0) {
        printf(RED "Invalid loan ID or loan not active!\n" RESET);
        waitForKeyPress();
        return;
    }

    printf("Outstanding balance: %.2f\n", loans[idx].totalPayable - loans[idx].amountRepaid);
    printf("Enter repayment amount: ");
    scanf("%f", &amount);

    loans[idx].amountRepaid += amount;
    recordTransaction(loans[idx].memberId, "Repayment", amount);

    if (loans[idx].amountRepaid >= loans[idx].totalPayable) {
        strcpy(loans[idx].status, "Closed");
        for (int i = 0; i < memberCount; i++) {
            if (members[i].id == loans[idx].memberId) {
                members[i].activeLoanId = -1;
                break;
            }
        }
        printf(GREEN "Loan fully repaid and closed!\n" RESET);
    } else {
        printf(GREEN "Repayment recorded. Remaining balance: %.2f\n" RESET,
               loans[idx].totalPayable - loans[idx].amountRepaid);
    }

    saveAllData();
    waitForKeyPress();
}

void viewMyLoanStatus() {
    if (currentMemberId == -1) { printf(RED "Please login first!\n" RESET); waitForKeyPress(); return; }
    showPageHeader("MY LOAN STATUS");

    int memberId = members[currentMemberId].id;
    int found = 0;

    for (int i = 0; i < loanCount; i++) {
        if (loans[i].memberId == memberId) {
            printf("Loan ID        : %d\n", loans[i].loanId);
            printf("Principal      : %.2f\n", loans[i].principal);
            printf("Interest Rate  : %.2f%%\n", loans[i].interestRate);
            printf("Total Payable  : %.2f\n", loans[i].totalPayable);
            printf("Amount Repaid  : %.2f\n", loans[i].amountRepaid);
            printf("Status         : %s\n", loans[i].status);
            printf("------------------------------------\n");
            found = 1;
        }
    }
    if (!found) printf("You have no loan records.\n");
    waitForKeyPress();
}

// ------------------------------------------------
// ADMIN / OFFICER MANAGEMENT
// ------------------------------------------------
void addNewOfficer() {
    if (strcmp(currentRole, "Admin") != 0) {
        printf(RED "Restricted to Administrators only!\n" RESET);
        waitForKeyPress();
        return;
    }
    showPageHeader("ADD NEW LOAN OFFICER");

    Member o;
    o.id = memberCount + 1;
    o.isActive = 1;
    o.savingsBalance = 0;
    o.activeLoanId = -1;
    strcpy(o.role, "Officer");

    printf("Enter Officer Name: ");
    scanf(" %[^\n]", o.name);

    do {
        printf("Enter NID (10 or 17 digits): ");
        scanf(" %[^\n]", o.nid);
    } while (!validateNID(o.nid) || findMemberByNID(o.nid) != -1);

    printf("Enter Password: ");
    scanf(" %[^\n]", o.password);
    printf("Enter Phone Number: ");
    scanf(" %[^\n]", o.phone);

    members[memberCount++] = o;
    saveAllData();

    printf(GREEN "Officer added successfully!\n" RESET);
    waitForKeyPress();
}

void listAllMembers() {
    showPageHeader("ALL MEMBERS");
    printf("%-5s %-20s %-15s %-10s %-12s\n", "ID", "Name", "Role", "Status", "Savings");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < memberCount; i++) {
        printf("%-5d %-20s %-15s %-10s %-12.2f\n",
               members[i].id, members[i].name, members[i].role,
               members[i].isActive ? "Active" : "Inactive", members[i].savingsBalance);
    }
    waitForKeyPress();
}

// ------------------------------------------------
// REPORTING
// ------------------------------------------------
void generateSystemReport() {
    showPageHeader("SYSTEM REPORT");

    int admins = 0, officers = 0, regular = 0, active = 0;
    for (int i = 0; i < memberCount; i++) {
        if (!members[i].isActive) continue;
        active++;
        if (strcmp(members[i].role, "Admin") == 0) admins++;
        else if (strcmp(members[i].role, "Officer") == 0) officers++;
        else regular++;
    }

    int pending = 0, disbursed = 0, closed = 0, rejected = 0;
    float totalDisbursed = 0, totalRepaid = 0;
    for (int i = 0; i < loanCount; i++) {
        if (strcmp(loans[i].status, "Pending") == 0) pending++;
        else if (strcmp(loans[i].status, "Disbursed") == 0) { disbursed++; totalDisbursed += loans[i].principal; }
        else if (strcmp(loans[i].status, "Closed") == 0) { closed++; totalDisbursed += loans[i].principal; }
        else if (strcmp(loans[i].status, "Rejected") == 0) rejected++;
        totalRepaid += loans[i].amountRepaid;
    }

    printf(CYAN "=== MEMBER STATISTICS ===\n" RESET);
    printf("Total Active Members : %d\n", active);
    printf("Administrators       : %d\n", admins);
    printf("Loan Officers        : %d\n", officers);
    printf("Regular Members      : %d\n\n", regular);

    printf(CYAN "=== LOAN STATISTICS ===\n" RESET);
    printf("Total Loans          : %d\n", loanCount);
    printf("Pending               : %d\n", pending);
    printf("Active (Disbursed)    : %d\n", disbursed);
    printf("Closed (Fully Repaid) : %d\n", closed);
    printf("Rejected              : %d\n\n", rejected);

    printf(CYAN "=== FINANCIAL SUMMARY ===\n" RESET);
    printf("Total Disbursed       : %.2f\n", totalDisbursed);
    printf("Total Repaid          : %.2f\n", totalRepaid);
    if (totalDisbursed > 0) {
        printf("Repayment Rate        : %.1f%%\n", (totalRepaid / totalDisbursed) * 100);
    }

    waitForKeyPress();
}

// ------------------------------------------------
// MENUS
// ------------------------------------------------
void showAdminMenu() {
    printf("1. View Profile\n2. List All Members\n3. Add New Officer\n");
    printf("4. Review Pending Loans\n5. Approve/Reject Loan\n6. Record Repayment\n");
    printf("7. Generate System Report\n8. Logout\n");
    printf(YELLOW "Enter your choice: " RESET);
}

void showOfficerMenu() {
    printf("1. View Profile\n2. Review Pending Loans\n3. Approve/Reject Loan\n");
    printf("4. Record Repayment\n5. Generate System Report\n6. Logout\n");
    printf(YELLOW "Enter your choice: " RESET);
}

void showMemberMenu() {
    printf("1. View Profile\n2. Deposit Funds\n3. Withdraw Funds\n");
    printf("4. Apply for Loan\n5. View My Loan Status\n6. Logout\n");
    printf(YELLOW "Enter your choice: " RESET);
}

void showMainMenu() {
    showPageHeader("MAIN MENU");
    if (currentMemberId == -1) {
        printf("1. Register New Member\n2. Login\n3. Exit System\n");
        printf(YELLOW "Enter your choice: " RESET);
    } else {
        printf(CYAN "Logged in as: %s | %s\n\n" RESET, members[currentMemberId].name, currentRole);
        if (strcmp(currentRole, "Admin") == 0) showAdminMenu();
        else if (strcmp(currentRole, "Officer") == 0) showOfficerMenu();
        else showMemberMenu();
    }
}

// ------------------------------------------------
// MAIN
// ------------------------------------------------
int main() {
    loadAllData();
    initializeSystem();
    showWelcomeBanner();

    int choice;
    while (1) {
        showMainMenu();
        scanf("%d", &choice);

        if (currentMemberId == -1) {
            switch (choice) {
                case 1: registerNewMember(); break;
                case 2: memberLogin(); break;
                case 3:
                    printf(YELLOW "\nThank you for using the Microfinance & Cooperative Management System!\n" RESET);
                    saveAllData();
                    exit(0);
                default:
                    printf(RED "Invalid choice!\n" RESET);
                    waitForKeyPress();
            }
        } else if (strcmp(currentRole, "Admin") == 0) {
            switch (choice) {
                case 1: showProfile(); break;
                case 2: listAllMembers(); break;
                case 3: addNewOfficer(); break;
                case 4: reviewPendingLoans(); break;
                case 5: approveOrRejectLoan(); break;
                case 6: recordRepayment(); break;
                case 7: generateSystemReport(); break;
                case 8:
                    currentMemberId = -1; strcpy(currentRole, "");
                    printf(YELLOW "Logged out.\n" RESET); waitForKeyPress();
                    break;
                default: printf(RED "Invalid choice!\n" RESET); waitForKeyPress();
            }
        } else if (strcmp(currentRole, "Officer") == 0) {
            switch (choice) {
                case 1: showProfile(); break;
                case 2: reviewPendingLoans(); break;
                case 3: approveOrRejectLoan(); break;
                case 4: recordRepayment(); break;
                case 5: generateSystemReport(); break;
                case 6:
                    currentMemberId = -1; strcpy(currentRole, "");
                    printf(YELLOW "Logged out.\n" RESET); waitForKeyPress();
                    break;
                default: printf(RED "Invalid choice!\n" RESET); waitForKeyPress();
            }
        } else {
            switch (choice) {
                case 1: showProfile(); break;
                case 2: depositFunds(); break;
                case 3: withdrawFunds(); break;
                case 4: applyForLoan(); break;
                case 5: viewMyLoanStatus(); break;
                case 6:
                    currentMemberId = -1; strcpy(currentRole, "");
                    printf(YELLOW "Logged out.\n" RESET); waitForKeyPress();
                    break;
                default: printf(RED "Invalid choice!\n" RESET); waitForKeyPress();
            }
        }
    }
    return 0;
}
