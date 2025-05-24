# 🛡️ MemGuard

**Guarding your memory, securing your code**

MemGuard is a lightweight memory leak detection tool for C/C++ programs. It dynamically intercepts heap memory operations to detect and report **memory leaks**, **double frees**, and **dangling pointer usage** during runtime — all without relying on external tools like Valgrind or AddressSanitizer.

---

## 🧠 Why MemGuard?

Manual memory management in C/C++ is error-prone. Common mistakes like:

* Memory leaks (memory not freed)
* Double frees (freeing already freed memory)
* Dangling pointers (using freed memory)

...can cause crashes, undefined behavior, and security flaws. MemGuard aims to make debugging these issues simple and intuitive.

---

## ⚙️ Features

✅ Tracks dynamic memory operations: `malloc`, `calloc`, `realloc`, `free`, `new`, `delete`
✅ Detects memory leaks, double frees, dangling pointers
✅ Supports array versions: `new[]`, `delete[]`
✅ Outputs structured logs in the console
✅ Minimal overhead, works with a simple `Makefile`
✅ Easy integration via macros (e.g., `MALLOC`, `FREE`, `NEW`, `DELETE`)
✅ Modular design with future GUI support

---

## 🧰 Technologies Used

* Language: **C++**
* Build: `Makefile`
* Core concepts: **Macros**, **Operator overloading**, **Linked lists**

---

## 📦 Installation & Setup

1. **Clone the Repository**

   ```bash
   git clone https://github.com/AkanshaRawat05/MemGuard.git
   cd MemGuard
   ```

2. **Build the Project**

   ```bash
   make
   ```

3. **Run a Test Program**

   ```bash
   ./main
   ```

4. **Review Logs**

   * Output will show memory allocations, deallocations, and any errors detected.

---

## 🔍 Example Output

```bash
[INFO] Allocated 40 bytes at main.cpp:25
[WARNING] Double free detected at main.cpp:39
[LEAK] 80 bytes not freed (main.cpp:17)
```

Or from the web interface:

![Memory Leak Report Screenshot](assets/report-screenshot.png)

* Total Allocations: 7
* Total Frees: 6
* Current Bytes Allocated: 4
* Peak Bytes Allocated: 44
* Issues detected: Double Free, Dangling Ptr, Leaked

Color-coded log table:

* 🔴 **Red**: Double Free / Leaked
* 🗾 **Orange**: Dangling Pointer
* 🟢 **Green**: Successfully freed

---

## 🧪 Testing

Test cases are included in the `more_testcases/` directory. They cover:

* Leaks
* Double frees
* Incorrect `new[]`/`delete` pairing
* Dangling pointer detection

---

## 📁 Directory Structure

```
MemGuard/
├── src/                 # Source code
├── test/                # Sample test programs
├── assets/              # Images for reports (e.g., screenshots)
├── Makefile             # Build file
├── README.md            # Project documentation
└── main.cpp             # Entry point with demo
```

---

## 🚀 Future Enhancements

* GUI or Web-based interface for visualization
* Thread safety and multithreaded program support
* More extensive test automation

---

## 👥 Authors

* [Akansha Rawat](mailto:akansha8230@gmail.com)
* Mansi Mehara
* Sneha Yadav
* Sneha Agarwal

---

## 📄 License

This project is for educational use and academic demonstration. Licensing info may be added later.
