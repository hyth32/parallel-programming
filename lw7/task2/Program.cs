using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Http;
using System.Threading.Tasks;

namespace task2
{
    class Program
    {
        static readonly HttpClient httpClient = new HttpClient();

        static async Task Main()
        {
            const string apiUrl = "https://dog.ceo/api/breeds/image/random";
            const int numberOfRequests = 10;

            await MeasureAsyncExecutionTime(apiUrl, numberOfRequests);
            MeasureSyncExecutionTime(apiUrl, numberOfRequests);
        }

        static async Task MeasureAsyncExecutionTime(string apiUrl, int numberOfRequests)
        {
            var stopwatch = Stopwatch.StartNew();

            await SendRequestsAsync(apiUrl, numberOfRequests);

            stopwatch.Stop();
            Console.WriteLine($"Asynchronous execution time: {stopwatch.ElapsedMilliseconds} ms");
        }

        static void MeasureSyncExecutionTime(string apiUrl, int numberOfRequests)
        {
            var stopwatch = Stopwatch.StartNew();

            SendRequestsSync(apiUrl, numberOfRequests).GetAwaiter().GetResult();

            stopwatch.Stop();
            Console.WriteLine($"Synchronous execution time: {stopwatch.ElapsedMilliseconds} ms");
        }

        static async Task SendRequestsAsync(string apiUrl, int numberOfRequests)
        {
            var tasks = new List<Task>();
            for (int i = 0; i < numberOfRequests; i++)
            {
                tasks.Add(GetAndProcessImageAsync(httpClient, apiUrl));
            }

            await Task.WhenAll(tasks);
        }

        static async Task SendRequestsSync(string apiUrl, int numberOfRequests)
        {
            for (int i = 0; i < numberOfRequests; i++)
            {
                await GetAndProcessImageAsync(httpClient, apiUrl);
            }
        }

        static async Task GetAndProcessImageAsync(HttpClient httpClient, string apiUrl)
        {
            try
            {
                Console.WriteLine($"Downloading image from URL: {apiUrl}");

                HttpResponseMessage response = await httpClient.GetAsync(apiUrl);

                if (response.IsSuccessStatusCode)
                {
                    string imageUrl = await response.Content.ReadAsStringAsync();
                    imageUrl = imageUrl.Replace("\\/", "/");
                    Console.WriteLine($"Image downloaded successfully from URL: {imageUrl}");
                }
                else
                {
                    Console.WriteLine($"Failed to get image from URL: {apiUrl}. Status code: {response.StatusCode}");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An error occurred: {ex.Message}");
            }
        }
    }
}

/* 
 * Объект Task: асинхронная операция, которая может быть выполнена асинхронно в фоновом потоке. 
 * Task.WhenAll: принимает коллекцию объектов Task и возвращает Task, который заврешится, когда все задачи коллекции завершатся. 
 * Похваляет эффективно ожидать завершения несколькиъ задач параллельно. 
 */