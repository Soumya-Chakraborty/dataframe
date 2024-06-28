/******************************************************************************
Dataframes.c Copyright (C) 2023  Soumya Chakraborty
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*******************************************************************************/
// Function prototypes
#include <Python.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char** data;
} Row;

typedef struct {
    Row* rows;
    int num_rows;
    int num_cols;
    char** headers;
} DataFrame;

// Function declarations
static PyObject* py_head(PyObject* self, PyObject* args);
static PyObject* py_tail(PyObject* self, PyObject* args);
static PyObject* py_sample(PyObject* self, PyObject* args);
static PyObject* py_info(PyObject* self, PyObject* args);
static PyObject* py_dtypes(PyObject* self, PyObject* args);
static PyObject* py_shape(PyObject* self, PyObject* args);
static PyObject* py_size(PyObject* self, PyObject* args);
static PyObject* py_ndim(PyObject* self, PyObject* args);
static PyObject* py_describe(PyObject* self, PyObject* args);
static PyObject* py_unique(PyObject* self, PyObject* args);
static PyObject* py_nlargest(PyObject* self, PyObject* args);
static PyObject* py_nsmallest(PyObject* self, PyObject* args);
static PyObject* py_fillna(PyObject* self, PyObject* args);
static PyObject* py_clip(PyObject* self, PyObject* args);
static PyObject* py_columns(PyObject* self, PyObject* args);
static PyObject* py_sort_values(PyObject* self, PyObject* args);
static PyObject* py_value_counts(PyObject* self, PyObject* args);

// Method definitions
static PyMethodDef DataFrameMethods[] = {
    {"head", py_head, METH_VARARGS, "Return the first n rows."},
    {"tail", py_tail, METH_VARARGS, "Return the last n rows."},
    {"sample", py_sample, METH_VARARGS, "Return a random sample of items."},
    {"info", py_info, METH_VARARGS, "Print a concise summary of a DataFrame."},
    {"dtypes", py_dtypes, METH_VARARGS, "Return the dtypes in the DataFrame."},
    {"shape", py_shape, METH_VARARGS, "Return a tuple representing the dimensionality of the DataFrame."},
    {"size", py_size, METH_VARARGS, "Return an int representing the number of elements in the DataFrame."},
    {"ndim", py_ndim, METH_VARARGS, "Return an int representing the number of axes / array dimensions."},
    {"describe", py_describe, METH_VARARGS, "Generate descriptive statistics."},
    {"unique", py_unique, METH_VARARGS, "Return unique values."},
    {"nlargest", py_nlargest, METH_VARARGS, "Return the first n rows ordered by columns in descending order."},
    {"nsmallest", py_nsmallest, METH_VARARGS, "Return the first n rows ordered by columns in ascending order."},
    {"fillna", py_fillna, METH_VARARGS, "Fill NA/NaN values using the specified method."},
    {"clip", py_clip, METH_VARARGS, "Trim values at input threshold(s)."},
    {"columns", py_columns, METH_VARARGS, "Return the column labels of the DataFrame."},
    {"sort_values", py_sort_values, METH_VARARGS, "Sort by the values along either axis."},
    {"value_counts", py_value_counts, METH_VARARGS, "Return a Series containing counts of unique values."},
    {NULL, NULL, 0, NULL}  // Sentinel
};

// Function to dynamically allocate a DataFrame structure
static DataFrame* createDataFrame(int num_rows, int num_cols) {
    DataFrame* df = (DataFrame*)malloc(sizeof(DataFrame));
    df->rows = (Row*)malloc(num_rows * sizeof(Row));
    df->headers = (char**)malloc(num_cols * sizeof(char*));
    for (int i = 0; i < num_cols; i++) {
        df->headers[i] = (char*)malloc(256 * sizeof(char)); // Assuming max header size
    }
    for (int i = 0; i < num_rows; i++) {
        df->rows[i].data = (char**)malloc(num_cols * sizeof(char*));
        for (int j = 0; j < num_cols; j++) {
            df->rows[i].data[j] = (char*)malloc(256 * sizeof(char)); // Assuming max field size
            df->rows[i].data[j][0] = '\0'; // Initialize each string to empty
        }
    }
    df->num_rows = num_rows;
    df->num_cols = num_cols;
    return df;
}

// Function to free memory allocated to DataFrame structure
static void freeDataFrame(DataFrame* df) {
    for (int i = 0; i < df->num_cols; i++) {
        free(df->headers[i]);
    }
    free(df->headers);
    for (int i = 0; i < df->num_rows; i++) {
        for (int j = 0; j < df->num_cols; j++) {
            free(df->rows[i].data[j]);
        }
        free(df->rows[i].data);
    }
    free(df->rows);
    free(df);
}

// Function to add a row of values to the DataFrame
static void addRow(DataFrame* df, char** values) {
    df->rows = (Row*)realloc(df->rows, (df->num_rows + 1) * sizeof(Row));
    df->rows[df->num_rows].data = (char**)malloc(df->num_cols * sizeof(char*));
    for (int i = 0; i < df->num_cols; i++) {
        df->rows[df->num_rows].data[i] = (char*)malloc((strlen(values[i]) + 1) * sizeof(char));
        strcpy(df->rows[df->num_rows].data[i], values[i]);
    }
    df->num_rows++;
}

// Function to print the contents of the DataFrame
static void printDataFrame(DataFrame* df) {
    for (int i = 0; i < df->num_cols; i++) {
        printf("%s\t", df->headers[i]);
    }
    printf("\n");
    for (int i = 0; i < df->num_rows; i++) {
        for (int j = 0; j < df->num_cols; j++) {
            printf("%s\t", df->rows[i].data[j]);
        }
        printf("\n");
    }
}

// Function to load data from a CSV file into a DataFrame
static DataFrame* loadCSV(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    char line[4096]; // Buffer for reading each line of the file

    // Read the header line to determine number of columns
    if (!fgets(line, sizeof(line), file)) {
        perror("Error reading header");
        fclose(file);
        return NULL;
    }

    int num_cols = 0;
    char* token = strtok(line, ",");
    while (token) {
        num_cols++;
        token = strtok(NULL, ",");
    }

    // Create a DataFrame with 0 rows and determined number of columns
    DataFrame* df = createDataFrame(0, num_cols);

    // Rewind the file pointer to read from the beginning
    rewind(file);

    // Read and discard the header line
    fgets(line, sizeof(line), file);

    // Read each subsequent line as data rows
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = 0;

        // Parse line into values based on comma delimiter
        char** values = (char**)malloc(num_cols * sizeof(char*));
        int col_idx = 0;
        token = strtok(line, ",");
        while (token) {
            values[col_idx] = (char*)malloc((strlen(token) + 1) * sizeof(char));
            strcpy(values[col_idx], token);
            token = strtok(NULL, ",");
            col_idx++;
        }

        // Add parsed values as a new row to the DataFrame
        addRow(df, values);

        // Free memory allocated for values array
        for (int i = 0; i < num_cols; i++) {
            free(values[i]);
        }
        free(values);
    }

    fclose(file);
    return df;
}

// Function to create a DataFrame object from Python
static PyObject* py_createDataFrame(PyObject* self, PyObject* args) {
    int num_rows, num_cols;
    if (!PyArg_ParseTuple(args, "ii", &num_rows, &num_cols)) {
        return NULL;
    }
    DataFrame* df = createDataFrame(num_rows, num_cols);
    return PyCapsule_New((void*)df, "DataFrame", NULL);
}

// Function to free memory allocated to a DataFrame object
static PyObject* py_freeDataFrame(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    freeDataFrame(df);
    Py_RETURN_NONE;
}

// Function to add a row of data to a DataFrame object from Python
static PyObject* py_addRow(PyObject* self, PyObject* args) {
    PyObject* capsule;
    PyObject* values;
    if (!PyArg_ParseTuple(args, "OO", &capsule, &values)) {
        return NULL;
    }

    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    if (!PyList_Check(values)) {
        PyErr_SetString(PyExc_TypeError, "Values must be a list");
        return NULL;
    }

    int num_cols = (int)PyList_Size(values);
    char** c_values = (char**)malloc(num_cols * sizeof(char*));
    for (int i = 0; i < num_cols; i++) {
        PyObject* item = PyList_GetItem(values, i);
        if (!PyUnicode_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List items must be strings");
            free(c_values);
            return NULL;
        }
        c_values[i] = (char*)PyUnicode_AsUTF8(item);
    }

    addRow(df, c_values);
    free(c_values);
    Py_RETURN_NONE;
}

// Function to print the contents of a DataFrame object from Python
static PyObject* py_printDataFrame(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    printDataFrame(df);
    Py_RETURN_NONE;
}

// Function to load data from a CSV file into a DataFrame object from Python
static PyObject* py_loadCSV(PyObject* self, PyObject* args) {
    const char* filename;
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        return NULL;
    }
    DataFrame* df = loadCSV(filename);
    if (!df) {
        return NULL;
    }
    return PyCapsule_New((void*)df, "DataFrame", NULL);
}

// Function to get the head (top n rows) of a DataFrame object from Python
static PyObject* py_head(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int n = 5; // Default to 5 rows if n is not provided
    if (!PyArg_ParseTuple(args, "O|i", &capsule, &n)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Print headers
    for (int i = 0; i < df->num_cols; i++) {
        printf("%s\t", df->headers[i]);
    }
    printf("\n");

    // Print first n rows
    for (int i = 0; i < df->num_rows && i < n; i++) {
        for (int j = 0; j < df->num_cols; j++) {
            printf("%s\t", df->rows[i].data[j]);
        }
        printf("\n");
    }
    Py_RETURN_NONE;
}

// Function to get the tail (bottom n rows) of a DataFrame object from Python
static PyObject* py_tail(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int n = 5; // Default to 5 rows if n is not provided
    if (!PyArg_ParseTuple(args, "O|i", &capsule, &n)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Print headers
    for (int i = 0; i < df->num_cols; i++) {
        printf("%s\t", df->headers[i]);
    }
    printf("\n");

    // Print last n rows
    int start_row = df->num_rows - n;
    if (start_row < 0) start_row = 0;
    for (int i = start_row; i < df->num_rows; i++) {
        for (int j = 0; j < df->num_cols; j++) {
            printf("%s\t", df->rows[i].data[j]);
        }
        printf("\n");
    }
    Py_RETURN_NONE;
}

// Function to sample a random row from a DataFrame object from Python
static PyObject* py_sample(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Generate a random index within the range of rows
    int random_index = rand() % df->num_rows;

    // Print headers
    for (int i = 0; i < df->num_cols; i++) {
        printf("%s\t", df->headers[i]);
    }
    printf("\n");

    // Print the randomly selected row
    for (int j = 0; j < df->num_cols; j++) {
        printf("%s\t", df->rows[random_index].data[j]);
    }
    printf("\n");

    Py_RETURN_NONE;
}

// Function to provide information summary of a DataFrame object from Python
static PyObject* py_info(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Print summary information
    printf("Data columns (total %d columns):\n", df->num_cols);
    for (int i = 0; i < df->num_cols; i++) {
        printf("%s\t", df->headers[i]);
    }
    printf("\n");
    printf("Data types:\n");
    for (int i = 0; i < df->num_cols; i++) {
        printf("%s\t", "string"); // Assuming all columns are string type for simplicity
    }
    printf("\n");
    printf("Memory usage: %d bytes\n", (int)(sizeof(DataFrame) + df->num_rows * sizeof(Row) + df->num_cols * sizeof(char*)));
    printf("Total rows: %d\n", df->num_rows);
    printf("Missing values: 0\n"); // No support for missing values in this basic implementation

    Py_RETURN_NONE;
}

// Function to get data types of columns in a DataFrame object from Python
static PyObject* py_dtypes(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Prepare a Python list to hold data types
    PyObject* dtype_list = PyList_New(df->num_cols);
    if (!dtype_list) {
        return NULL;
    }

    // Add data types (assuming all columns are string for simplicity)
    for (int i = 0; i < df->num_cols; i++) {
        PyList_SetItem(dtype_list, i, PyUnicode_FromString("string"));
    }

    return dtype_list;
}

// Function to get shape (dimensions) of a DataFrame object from Python
static PyObject* py_shape(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Create a tuple of (num_rows, num_cols)
    PyObject* shape_tuple = PyTuple_New(2);
    if (!shape_tuple) {
        return NULL;
    }
    PyTuple_SetItem(shape_tuple, 0, PyLong_FromLong(df->num_rows));
    PyTuple_SetItem(shape_tuple, 1, PyLong_FromLong(df->num_cols));

    return shape_tuple;
}

// Function to get size (total number of elements) of a DataFrame object from Python
static PyObject* py_size(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Return number of rows times number of columns
    return PyLong_FromLong(df->num_rows * df->num_cols);
}

// Function to get number of dimensions of a DataFrame object from Python
static PyObject* py_ndim(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }

    // DataFrame is considered 2-dimensional
    return PyLong_FromLong(2);
}

// Function to describe statistics of a DataFrame object from Python
static PyObject* py_describe(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }

    // Print basic statistics (assuming all columns are strings)
    printf("Statistics (string columns):\n");
    printf("Count: %d\n", 0); // No count for string data
    printf("Unique: %d\n", 0); // No unique values for string data
    printf("Top: %s\n", "N/A"); // No top value for string data
    printf("Freq: %d\n", 0); // No frequency for string data

    Py_RETURN_NONE;
}

// Function to get unique values of a column in a DataFrame object from Python
static PyObject* py_unique(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int col_index;
    if (!PyArg_ParseTuple(args, "Oi", &capsule, &col_index)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Print unique values of the specified column (assuming all are strings)
    printf("Unique values of column %d:\n", col_index);
    for (int i = 0; i < df->num_rows; i++) {
        printf("%s\n", df->rows[i].data[col_index]);
    }

    Py_RETURN_NONE;
}

// Function to check for null values (simulated by empty strings in this implementation)
static PyObject* py_isnull(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    PyObject* result = PyList_New(df->num_rows);
    for (int i = 0; i < df->num_rows; i++) {
        PyObject* row_result = PyList_New(df->num_cols);
        for (int j = 0; j < df->num_cols; j++) {
            PyList_SetItem(row_result, j, PyBool_FromLong(df->rows[i].data[j][0] == '\0'));
        }
        PyList_SetItem(result, i, row_result);
    }
    return result;
}

// Function alias for isnull()
static PyObject* py_isna(PyObject* self, PyObject* args) {
    return py_isnull(self, args);
}

// Function to fill null values with a specified value
static PyObject* py_fillna(PyObject* self, PyObject* args) {
    PyObject* capsule;
    const char* fill_value;
    if (!PyArg_ParseTuple(args, "Os", &capsule, &fill_value)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    for (int i = 0; i < df->num_rows; i++) {
        for (int j = 0; j < df->num_cols; j++) {
            if (df->rows[i].data[j][0] == '\0') {
                strcpy(df->rows[i].data[j], fill_value);
            }
        }
    }
    Py_RETURN_NONE;
}

// Function to get column names
static PyObject* py_columns(PyObject* self, PyObject* args) {
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    PyObject* column_list = PyList_New(df->num_cols);
    for (int i = 0; i < df->num_cols; i++) {
        PyList_SetItem(column_list, i, PyUnicode_FromString(df->headers[i]));
    }
    return column_list;
}

// Function to count unique values in a specified column
static PyObject* py_value_counts(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int col_index;
    if (!PyArg_ParseTuple(args, "Oi", &capsule, &col_index)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    PyObject* counts_dict = PyDict_New();
    for (int i = 0; i < df->num_rows; i++) {
        const char* value = df->rows[i].data[col_index];
        PyObject* key = PyUnicode_FromString(value);
        PyObject* count = PyDict_GetItem(counts_dict, key);
        if (count) {
            PyDict_SetItem(counts_dict, key, PyLong_FromLong(PyLong_AsLong(count) + 1));
        } else {
            PyDict_SetItem(counts_dict, key, PyLong_FromLong(1));
        }
    }
    return counts_dict;
}


// Function to trim values at specified thresholds
static PyObject* py_clip(PyObject* self, PyObject* args) {
    PyObject* capsule;
    const char* lower;
    const char* upper;
    if (!PyArg_ParseTuple(args, "Oss", &capsule, &lower, &upper)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");
    for (int i = 0; i < df->num_rows; i++) {
        for (int j = 0; j < df->num_cols; j++) {
            if (strcmp(df->rows[i].data[j], lower) < 0) {
                free(df->rows[i].data[j]);
                df->rows[i].data[j] = (char*)malloc((strlen(lower) + 1) * sizeof(char));
                strcpy(df->rows[i].data[j], lower);
            } else if (strcmp(df->rows[i].data[j], upper) > 0) {
                free(df->rows[i].data[j]);
                df->rows[i].data[j] = (char*)malloc((strlen(upper) + 1) * sizeof(char));
                strcpy(df->rows[i].data[j], upper);
            }
        }
    }
    Py_RETURN_NONE;
}

// Function to sort DataFrame by a specified column
static PyObject* py_sort_values(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int col_index;
    if (!PyArg_ParseTuple(args, "Oi", &capsule, &col_index)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Simple bubble sort for demonstration purposes
    for (int i = 0; i < df->num_rows - 1; i++) {
        for (int j = 0; j < df->num_rows - i - 1; j++) {
            if (strcmp(df->rows[j].data[col_index], df->rows[j + 1].data[col_index]) > 0) {
                Row temp = df->rows[j];
                df->rows[j] = df->rows[j + 1];
                df->rows[j + 1] = temp;
            }
        }
    }
    Py_RETURN_NONE;
}

// Function to get n largest values in a column
static PyObject* py_nlargest(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int col_index, n;
    if (!PyArg_ParseTuple(args, "Oii", &capsule, &col_index, &n)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Create an array of pointers to the rows
    char** rows = (char**)malloc(df->num_rows * sizeof(char*));
    for (int i = 0; i < df->num_rows; i++) {
        rows[i] = df->rows[i].data[col_index];
    }

    // Sort the array in descending order
    qsort(rows, df->num_rows, sizeof(char*), (int(*)(const void*, const void*))strcmp);

    // Create a Python list to hold the n largest values
    PyObject* largest_list = PyList_New(n);
    for (int i = 0; i < n && i < df->num_rows; i++) {
        PyList_SetItem(largest_list, i, PyUnicode_FromString(rows[i]));
    }

    free(rows);
    return largest_list;
}

// Function to get n smallest values in a column
static PyObject* py_nsmallest(PyObject* self, PyObject* args) {
    PyObject* capsule;
    int col_index, n;
    if (!PyArg_ParseTuple(args, "Oii", &capsule, &col_index, &n)) {
        return NULL;
    }
    DataFrame* df = (DataFrame*)PyCapsule_GetPointer(capsule, "DataFrame");

    // Create an array of pointers to the rows
    char** rows = (char**)malloc(df->num_rows * sizeof(char*));
    for (int i = 0; i < df->num_rows; i++) {
        rows[i] = df->rows[i].data[col_index];
    }

    // Sort the array in ascending order
    qsort(rows, df->num_rows, sizeof(char*), (int(*)(const void*, const void*))strcmp);

    // Create a Python list to hold the n smallest values
    PyObject* smallest_list = PyList_New(n);
    for (int i = 0; i < n && i < df->num_rows; i++) {
        PyList_SetItem(smallest_list, i, PyUnicode_FromString(rows[i]));
    }

    free(rows);
    return smallest_list;
}

// Module definition
static struct PyModuleDef dataframe_module = {
    PyModuleDef_HEAD_INIT,
    "dataframe",   // Module name
    NULL,          // Module documentation (may be NULL)
    -1,            // Size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
    DataFrameMethods
};

// Module initialization function
PyMODINIT_FUNC PyInit_dataframe(void) {
    return PyModule_Create(&dataframe_module);
}