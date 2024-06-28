# test.py

import dataframe

# Example usage
def main():
    # Create a DataFrame with 3 rows and 4 columns
    df = dataframe.createDataFrame(3, 4)
    dataframe.addRow(df, ["John", "Doe", "30", "Engineer"])
    dataframe.addRow(df, ["Jane", "Smith", "25", "Doctor"])
    dataframe.addRow(df, ["Tom", "Brown", "40", "Lawyer"])

    # Print DataFrame contents
    print("DataFrame contents:")
    dataframe.printDataFrame(df)

    # Get and print the head of the DataFrame
    print("\nHead of DataFrame:")
    dataframe.head(df, 2)

    # Get and print the tail of the DataFrame
    print("\nTail of DataFrame:")
    dataframe.tail(df, 1)

    # Sample a random row from the DataFrame
    print("\nSample from DataFrame:")
    dataframe.sample(df)

    # Print information summary of the DataFrame
    print("\nDataFrame information:")
    dataframe.info(df)

    # Get data types of columns
    print("\nData types of columns:")
    print(dataframe.dtypes(df))

    # Get shape of the DataFrame
    print("\nShape of DataFrame:")
    print(dataframe.shape(df))

    # Get size of the DataFrame
    print("\nSize of DataFrame:")
    print(dataframe.size(df))

    # Get number of dimensions of the DataFrame
    print("\nNumber of dimensions of DataFrame:")
    print(dataframe.ndim(df))

    # Describe statistics of the DataFrame
    print("\nDescribe DataFrame:")
    dataframe.describe(df)

    # Get unique values of a column
    print("\nUnique values of column 0:")
    dataframe.unique(df, 0)

    # Free DataFrame memory
    dataframe.freeDataFrame(df)

if __name__ == "__main__":
    main()
