using System;
using System.Threading;
using WebSocketSharp.Server;

namespace PluginDevServer
{
    class Program
    {
        static void Main(string[] args)
        {
            var server = new WebSocketServer(4649);
            server.AddWebSocketService<BroadcastService>("/PluginDevHelper");
            server.Start();
            if (server.IsListening)
            {
                Console.WriteLine("Listening on port {0}, and providing WebSocket services:", server.Port);
                foreach (var path in server.WebSocketServices.Paths)
                    Console.WriteLine("- {0}", path);
            }

            Console.WriteLine("\nPress Ctrl+C to kill the server...");
            Thread.Sleep(Timeout.Infinite);

            server.Stop();
        }
    }
}
