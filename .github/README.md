# Neva

**Neva** is a Chromium‑based application runtime and browser framework designed for embedded devices
manufactured by LG Electronics. It provides a powerful, customizable environment for running
web applications on a wide variety of hardware targets.

Neva focuses on portability, device‑specific extensibility, and ease of integration
with platform‑level components such as app managers, window managers, and input systems.

---

## ✨ Key Features

### 🚀 Web Application Runtime
- Provides a robust runtime environment for launching and managing web applications.
- Offers integration interfaces for platform app managers (start / suspend / resume / stop).
- Supports communication with window managers for fullscreen or fixed‑position rendering and
  z‑order control.

### 🔌 Device‑Specific Extensions
- Acts as a flexible framework for adding custom Web APIs tailored to device capabilities.
- Designed to accommodate platform‑specific requirements without modifying Chromium’s core
  structure.

### 🖼 Graphics Backend (Ozone‑Wayland)
- Includes a graphics backend built on **ozone-wayland**, enabling efficient rendering
  on embedded Linux systems.

### 🧭 Browser Shell
- Provides a versatile Browser Shell that can be used both for launching a specific web page
  or web link and for running a reference Web browser.

### 🧪 Web Application Manager (WAM) emulator
- Includes a WAM demo emulator (simply named **wam-demo**) for testing
  Neva’s application runtime and browser behavior on development machines.

### ⌨️ Input Device Support
- Keyboard
- Virtual keyboard
- Mouse
- Touch input

---

## 🧬 Branch Structure

Neva uses a two‑branch development model:

### **`neva-main`** – Primary Development Branch
- Contains general development: improvements, refactoring, bug fixes, and new features.
- Chromium version updated only on **major Chromium version changes**
  (through merges from `neva-main-rolling`).

### **`neva-main-rolling`** – Rolling Chromium Update Branch
- Continuously updated to track upstream Chromium changes.
- Represents the most **current** and **Chromium‑aligned** version of Neva.
- Serves as the source of truth for merging major updates into `neva-main`.

---

## 📦 How to build
Instruction for building Neva for webOS/OSE target can be found in the [meta-neva](https://github.com/nevaoss/meta-neva) repository.
