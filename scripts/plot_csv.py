import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import io
import sys # Import the sys module to access command-line arguments

def _plot_dataframe(df: pd.DataFrame, title: str, save_path: str):
    """
    Internal function to plot a DataFrame.
    Assumes DataFrame has 'type', 'time', and 'count' columns.
    """
    print("Preview of data to be plotted:")
    print(df.head())
    print("\nData information:")
    df.info()

    plt.figure(figsize=(12, 7))
    sns.lineplot(
        data=df,
        x='time',
        y='count',
        hue='type',
        marker='o',
        linewidth=2
    )

    plt.title(title, fontsize=16)
    plt.xlabel('Time', fontsize=12)
    plt.ylabel('Count', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(title='Data Type', bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=300)
        print(f"\nChart saved to: {save_path}")
    else:
        plt.show()

def plot_data_from_string(data_string: str, title: str = 'Count Over Time for Data Types', save_path: str = None):
    """
    Parses the given string data and plots a line chart.

    Args:
        data_string (str): A multi-line string containing data type, time, and count.
                           Each line should be in the format: "data_type, time, count".
        title (str): The title of the chart.
        save_path (str, optional): If provided, the chart will be saved to this path. E.g., "my_plot.png".
                                   Defaults to None, which means the chart will be displayed.
    """
    if not data_string.strip():
        print("Error: The provided data string is empty.")
        return

    try:
        df = pd.read_csv(io.StringIO(data_string), header=None, names=['type', 'time', 'count'])
    except Exception as e:
        print(f"Error parsing data from string: {e}")
        print("Please ensure the data format is 'data_type, time, count' and that time/count are numeric.")
        return

    _plot_dataframe(df, title, save_path)


def plot_data_from_file(file_path: str, title: str = 'Count Over Time for Data Types', save_path: str = None):
    """
    Reads data from a specified file, parses it, and plots a line chart.

    Args:
        file_path (str): The path to the data file.
                         Each line should be in the format: "data_type, time, count".
        title (str): The title of the chart.
        save_path (str, optional): If provided, the chart will be saved to this path. E.g., "my_plot.png".
                                   Defaults to None, which means the chart will be displayed.
    """
    if not file_path:
        print("Error: No file path provided.")
        return

    try:
        df = pd.read_csv(file_path, header=None, names=['type', 'time', 'count'])
    except FileNotFoundError:
        print(f"Error: File not found at '{file_path}'. Please check the path and ensure it's correct.")
        return
    except Exception as e:
        print(f"Error parsing data from file '{file_path}': {e}")
        print("Please ensure the file exists and the data format is 'data_type, time, count' and that time/count are numeric.")
        return

    _plot_dataframe(df, title, save_path)

# Example Data (for string plotting, if no file is provided)
slab_data_string = """
slab_a, 0, 54
slab_a, 1, 930
slab_a, 2, 54
slab_a, 4, 349
slab_a, 6, 54
slab_a, 8, 853
slab_a, 10, 72
slab_a, 12, 1029
slab_a, 14, 72
slab_a, 16, 58
slab_a, 90, 72
slab_b, 0, 51
slab_b, 1, 99
slab_b, 2, 51
slab_b, 4, 328
slab_b, 6, 51
slab_b, 8, 832
slab_b, 10, 51
slab_b, 12, 819
slab_b, 14, 51
slab_b, 16, 37
slab_b, 90, 51
slab_c, 0, 51
slab_c, 1, 100
slab_c, 2, 52
slab_c, 4, 329
slab_c, 6, 52
slab_c, 8, 833
slab_c, 10, 60
slab_c, 12, 820
slab_c, 14, 52
slab_c, 16, 43
slab_c, 90, 64
"""

if __name__ == "__main__":
    # Ensure all required libraries are installed.
    # If not installed, run:
    # pip install pandas matplotlib seaborn

    # Check if a file path is provided as a command-line argument
    if len(sys.argv) > 1:
        file_name = sys.argv[1] # The first argument after the script name is the file path
        print(f"Attempting to plot data from file: {file_name}")
        plot_data_from_file(file_name, title=f'Slab Count (from {file_name})', save_path=f'plot_from_{file_name}.png')
    else:
        print("No file path provided as a command-line argument.")
        print("Usage: python your_script_name.py <your_data_file.csv>")
        print("\nPlotting from internal string data as an example instead.")
        # Fallback to plotting from the internal string data if no file is specified
        plot_data_from_string(slab_data_string, title='Slab Count (from internal string)')

        # Optionally, create a dummy file for demonstration purposes if running without arguments
        # file_name = "example_slab_data.csv"
        # try:
        #     with open(file_name, "w") as f:
        #         f.write(slab_data_string.strip())
        #     print(f"\nCreated dummy data file: {file_name} for future use.")
        # except Exception as e:
        #     print(f"Could not create dummy file {file_name}: {e}")
