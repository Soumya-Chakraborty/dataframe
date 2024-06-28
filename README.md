# DataFrame Implementation in C with Python Bindings

This project provides a basic implementation of a DataFrame in C with Python bindings using ctypes. It allows efficient handling and manipulation of tabular data within Python, leveraging the performance benefits of C.

## Features

- **DataFrame Structure**: Implements a simple DataFrame structure in C, supporting operations like columnar operations, row operations, and basic data manipulations.
- **Python Bindings**: Provides seamless integration with Python using ctypes, allowing DataFrame objects to be instantiated and manipulated directly from Python code.
- **Efficient Performance**: Leveraging C for critical operations ensures efficient performance, especially for large datasets and computationally intensive tasks.

## Installation

### Prerequisites

- Python 3.x
- C compiler (e.g., GCC)

### Installation Steps

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/dataframes.git
   cd dataframes
   ```

2. Build and install the package:

   ```bash
   python setup.py install
   ```

   This will compile the C extension and install the package along with its dependencies.

## Usage

Here's a quick example of how to use the `DataFrame` from Python:

```python
from dataframes import DataFrame

# Create a new DataFrame
df = DataFrame()

# Add columns
df.add_column("Name", ["Alice", "Bob", "Charlie"])
df.add_column("Age", [25, 30, 28])

# Print the DataFrame
print("DataFrame:")
print(df)

# Accessing data
print("First row:")
print(df.get_row(0))

# Modifying data
df.set_value(1, "Age", 31)
print("Updated Age of Bob:")
print(df.get_row(1))
```

## Contributing

Contributions are welcome! If you'd like to contribute to this project, feel free to fork the repository and submit a pull request.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Contact

For any inquiries or issues, please open an issue on the [GitHub repository](https://github.com/yourusername/dataframes/issues).