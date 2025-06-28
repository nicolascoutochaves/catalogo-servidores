# 📊 Catálogo de Salários dos Servidores Públicos

A command‑line tool written in C to import, index, query and analyze municipal public servants' salary data. Supports multiple city profiles, automatic CSV preprocessing (encoding, header sanitization), efficient binary storage with B‑tree‑style indices, statistical summaries and terminal‐based Gaussian distribution plots.

## 📂 Repository Structure

```
├── bin/                         # Compiled binaries
├── build/                       # Object files and executables by platform
├── data/                        # Raw CSV files
├── profiles/                    # JSON profiles for each municipality
│   ├── gravatai.json
│   └── sao_paulo.json
├── index/                       # Generated index files (.idx)
├── src/                         # C source code
│   ├── csv.c                    # CSV parsing & preprocessing
│   ├── preprocess.c             # File sanitization (encoding → ASCII)
│   ├── json_profile.c           # Load field aliases from JSON
│   ├── sort_utils.c             # Sorting & index building
│   ├── index.c                  # High‑level index builder
│   ├── search.c                 # Indexed search (by ID/name)
│   ├── stats.c                  # Statistical & Gaussian routines
│   └── main.c                   # CLI entry point
├── include/                     # Public headers
├── tests/                       # Sample CSV & profile
└── Makefile                     # Build instructions
```

## 🚀 Features

* **Multiple Profiles**: JSON files describe header aliases per city (ex: `"Nome"`, `"Funcionário"`, etc.).
* **Preprocessing**: Converts CSV to ASCII, normalizes headers, handles UTF‑8 & ISO‑8859‑1.
* **Binary Storage**: Compact `public_employees.dat` for fast I/O.
* **Dynamic Indexing**: Builds `.idx` files (by ID, name, gross/net salary) using efficient radix and quicksort.
* **Indexed Search**: Instant lookup by employee ID or exact name via binary search over index.
* **Sorting**: Terminal‑side sorted print, ascending/descending.
* **Statistics**: Mean, variance, standard deviation and Gaussian distribution ranges (30%, 50%, 80%, 90%, 95%).
* **Performance Metrics**: Reports timings for each phase with millisecond precision.

## ⚙️ Installation

1. Clone the repo:

   ```bash
   git clone https://github.com/yourusername/publicservant-salary-explorer.git
   cd publicservant-salary-explorer
   ```
2. Edit or add city profiles in `profiles/` (refer to examples).
3. Build for Linux or Windows:

   ```bash
   make all
   ```

## 💡 Usage

```bash
# Basic invocation
./build/linux/app \  # or .exe on Windows
  data/estado_sao_paulo.csv \  # raw CSV
  profiles/sampa.json         # JSON profile

# Optional: specify output binary
./build/linux/app data/gravatai.csv profiles/gravatai.json bin/gravatai.dat
```

This will:

1. Load the JSON profile and report encoding.
2. Preprocess the CSV (sanitize, normalize headers).
3. Convert CSV to a binary `.dat` file.
4. Print all records and key statistics.
5. Build four index files in `index/`:

   * `<basename>_id.idx`
   * `<basename>_name.idx`
   * `<basename>_gross_salary.idx`
   * `<basename>_net_salary.idx`

## 🔍 Searching

Once indices are built, you can query by ID or name with the `search` utility:

```c
search_by_id("bin/public_employees.dat", "index/gravatai_id.idx", 12345);
search_by_name("bin/public_employees.dat", "index/gravatai_name.idx", "JOAO SILVA");
```

## 📄 JSON Profile Format

Each profile lists header aliases and encoding:

```json
{
  "fields": {
    "id":       ["Funcionário", "Matrícula"],
    "name":     ["Nome", "Nome Funcionário"],
    "position": ["Cargo", "Função"],
    "dept":     ["Centro Custo", "Órgão", "Setor"],
    "gross":    ["Salário Bruto", "Remuneração Total"],
    "discount": ["Descontos", "Deduções"],
    "net":      ["Salário Líquido", "Remuneração Após Deduções"]
  },
  "encoding": "ISO-8859-1"
}
```

Adapt this JSON for each municipality's CSV header conventions.

## 🛠️ Contributing

PRs welcome! Please:

1. Fork and branch.
2. Add tests for new profiles.
3. Ensure Valgrind clean (zero errors & leaks).
4. Update documentation.

## 📜 License

MIT © Your Name

---

*Happy analyzing public sector salaries!*
