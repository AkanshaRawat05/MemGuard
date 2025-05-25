# 🛡️ MemGuard

**Guarding your memory, securing your code**

MemGuard is a lightweight memory tracking and leak detection tool for C and C++ applications. Designed to tackle the challenges of manual memory management, MemGuard replaces standard allocation functions with custom hooks to monitor and log all dynamic memory operations (malloc, calloc, realloc, free, new, and delete) during program execution.

---

## 🧠 Why MemGuard?

In C/C++, developers are responsible for explicitly allocating and freeing memory. This often leads to difficult-to-debug errors such as:

Memory leaks – allocated memory never freed

Double frees – freeing the same memory twice

Dangling pointers – accessing memory after it's been freed

Incorrect use of new[] / delete[]

These bugs can cause program crashes, instability, and security vulnerabilities, especially in small projects or for beginners.

---

## ⚙️ Features

MemGuard provides a developer-friendly alternative to complex tools like Valgrind and AddressSanitizer:

✅ Custom Memory Tracking Library
Hooks into memory operations using macros and operator overloading.

📊 Clear Diagnostic Reports
Outputs both JSON and CSV reports for easy visualization and automation.

🌐 Interactive Web Dashboard
Visualizes memory operations with categorized tags:

   -🔴 Memory Leaks/ Double Frees

   -🟡 Dangling Pointers

   -🟢 Valid Frees

✅ Seamless File  Analysis
 The backend processes files via run_all.sh and displays results.

🧩 Minimal Integration Overhead
Lightweight, easy to plug into existing codebases without heavy performance costs.

🔮 Scalable Architecture
While currently single-threaded, MemGuard is structured for easy extension and future multithreading support.

---

## 🧰 Technologies Used


| **Technology**          | **Purpose**                                                                 |
| **C/C++**               | Core language for implementing memory tracking logic (tracker.cpp).         |
| **HTML/CSS/JavaScript** | Used to build the interactive frontend dashboard for visualizing reports.   |
| **json.hpp (nlohmann)** | Third-party C++ library to generate structured JSON output from memory data.|
| **Bash (run\_all.sh)**  | Automates the compilation and execution flow along with report generation.  |
| **Makefile**            | Simplifies compilation process for tracker and user-provided C/C++ files.   |


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
   ./leak_detector
   ```
4.**Run frontend**
```bash
   python3 -m http.server 8000
```

5. **Review Logs**

   * Output will show memory allocations, deallocations, and any errors detected.

---

## 🔍 Example Output
 from the web interface:

![Memory Leak Report Screenshot](assets/output.jpg)

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

🧵 Multithreading Support
Extend MemGuard to track memory operations safely across multiple threads, ensuring accurate detection in concurrent environments.

🧠 Static Analysis Integration
Combine dynamic tracking with static code analysis to catch potential memory issues before runtime.

🧩 IDE Plugins
Develop plugins for popular IDEs (e.g., Visual Studio, CLion) to provide real-time memory diagnostics during coding.

📈 Improved Visualization
Add timeline views and detailed call stacks to help trace the origin of memory errors more precisely.

🌐 Support for More Languages
Expand support beyond C/C++ to include other languages with manual memory management needs.

⚡ Performance Optimization
Further reduce runtime overhead to enable use in performance-critical applications.

🔁 Automated CI/CD Integration
Integrate MemGuard reports into continuous integration pipelines for automatic memory safety checks on every build.

---

## 👥 Authors

* [Akansha Rawat](mailto:akansha8230@gmail.com)
* Mansi Mehara
* Sneha Yadav
* Sneha Agarwal

---

## 📄 License

This project is for educational use and academic demonstration. Licensing info may be added later.


