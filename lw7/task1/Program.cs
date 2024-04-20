using System;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace task1
{
    class Program
    {
        static async Task Main(string[] args)
        {
            Console.WriteLine("Input file path:");
            string filePath = Console.ReadLine();

            Console.WriteLine("Input chars to remove (w/o spaces):");
            string charsToRemove = Console.ReadLine();

            var strBuilder = new StringBuilder();

            using (var reader = new StreamReader(filePath))
            {
                string line;
                while ((line = await reader.ReadLineAsync()) != null)
                {
                    foreach (char c in charsToRemove)
                    {
                        line = line.Replace(c.ToString(), "");
                    }
                    strBuilder.AppendLine(line);
                }
            }

            using (var writer = new StreamWriter(filePath))
            {
                await writer.WriteAsync(strBuilder.ToString());
            }

            Console.WriteLine("Изменения сохранены.");
            Console.ReadLine();
        }
    }
}
