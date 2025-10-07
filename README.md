# FinTrack Pro — Module 2 OEP (C++)

- Name: Tim Mäser

## Summary
- Purpose: Console-based personal finance tracker create Savings/Current/Credit Accounts where you can  add transactions, list by month, compute totals, search notes, do budgets, make transfers, search for transfers.
- Flow: Users chooses what action to persue. Initially users will want to create a account or load one, then add a transaction by entering an ID, date, amount, category, and note for a transaction, then browse summaries by month or search by keyword.
- Structure: Directories and Separation into small classes with simple helpers for formatting, validation, and aggregation.

## Module 2 Concepts Used
- Has-A / Is-A relationships
  - `User` to `Account` (polymorphic container used when listing accounts or computing net worth).
  - `Account` to `Ledger` (each account keeps its own transaction histor, leveraged for balance queries and CSV sync).
  - `Account` base class with `CheckingAccount`, `SavingsAccount`, `CreditAccount` overrides (different balance behavior + extra state).
- File I/O
  - `readCSV` / `writeLedger` / `appendTransaction` manage CSV implementations as specified by requirements
- Extra Supporting abstractions: `Budget` for category limits/variance, `RecurringTransaction` for expanded scheduling, `TransferTransaction` for mirrored debits/credits between accounts (Is-A Relationship).

## Citations
- `main.cpp`: Line 43: https://stackoverflow.com/questions/25020129/cin-ignorenumeric-limitsstreamsizemax-n

## Future Work
- Add a small automated testing for the CSV input/output
- Add the account specific data that would be accurate such as no overdrafts, fees on any amount not paid off, etc.
- Improve recurring scheduling so weekly/daily instances respect the actual calendar instead of the simplified 28-day assumption.
- More analytics (maybe charts) on the transactions.

## Help for Examiner/TA
If you want to use the sample data that is in the directory
`/SampleDataForExaminer`
* If you are using clion, move it into the cmake-build-debug folder
  * Otherwise make your own current/savings/credit account and copy the data into the corresponding files, making sure that you adjust the account names in the files

* To run normal things you must first create/load a account with correct Account name. Then you can use the filters, etc for that account.
  * If you want to use cross-account features like transfer you must load another Account by returning to the main menu and loading the account similarly to the first time.

* To create new csv files you must save to file and stop the run for the file to show up.

### Module 3 Features:
* Multi-Account Support (Credit, Savings, Current)
  * Currently there is no special constraints for each due to limited time available (however classes like monthly interest are already implemented)
* File input and output to save data about accounts, meaning people dont have to input all their transactions everytime like it was in module 1.
* Ability to set budgets and see budget variance per category
* Recurring transactions (monthly/weekly/daily) and transfer transactions (between accounts)

## Expected Grade
- Main program - Expecting 40 for all the effort and extra user input added to the class in comparison to module 1
- Relationship — Expected 60-100 points (3 different classes + transaction/transfer Is-A)
- File I/O - 10 points since I only use one, but can generate multiple files with data and load multiple files (so maybe more bonus points?)
- Testing - 11 points (1 point per test perhaps).
- Realistic: 40 + 80 + 10 + 10 + 11 = 151 points
- Optimistic 40 + 100 + 15 + 20 + 15 = 190 points

## Known Bugs / Issues
* Currently, the recurring transactions will set it all on one date due to the differing lengths of a each month
* No input validation on csv input, if csv has invalid input, it will just crash. However this will never happen if the csv file is created using the program.
