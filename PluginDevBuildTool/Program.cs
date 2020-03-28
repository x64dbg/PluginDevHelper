using System;
using System.Threading;
using System.IO;
using WebSocketSharp;

namespace PluginDevBuildTool
{
    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine(@"Usage: PluginDevBuildTool unload/reload ""c:\full\path\to\plugin.dp64""");
                return 1;
            }

            var action = args[0];
            switch (action)
            {
                case "unload":
                case "reload":
                    break;
                default:
                    Console.WriteLine($"PluginDevBuildTool: Invalid action \"{action}\"!");
                    return 1;
            }

            var pluginPath = args[1];
            var pluginName = Path.GetFileName(pluginPath);
            var fileInfo = new FileInfo(pluginPath);
            if (!fileInfo.Exists)
            {
                Console.WriteLine($"PluginDevBuildTool: \"{pluginPath}\" does not exist!");
                return 1;
            }

            bool CanOpenExclusively()
            {
                try
                {
                    using (var fs = fileInfo.Open(FileMode.Open, FileAccess.ReadWrite, FileShare.None))
                    {
                    }
                    return true;
                }
                catch
                {
                    return false;
                }
            }

            bool BroadcastIntention()
            {
                var result = true;
                using (var ws = new WebSocket("ws://127.0.0.1:4649/PluginDevHelper"))
                {
                    ws.OnError += (sender, e) =>
                    {
                        result = false;
                    };
                    ws.Log.Output = (d, s) =>
                    {
                    };
                    ws.Connect();
                    if(!ws.IsAlive)
                        return false;
                    ws.Send($"{action}:{pluginName}");
                }
                return result;
            }

            var unloadedPath = pluginPath + ".unloaded";

            if (action == "unload")
            {
                // Nobody has this plugin loaded
                if (CanOpenExclusively())
                {
                    Console.WriteLine($"PluginDevBuildTool: Exclusive access to {pluginName} already obtained!");
                    return 0;
                }

                Console.WriteLine($"PluginDevBuildTool: Failed to obtain exclusive access, requesting to unload {pluginName}");
                if (!BroadcastIntention())
                {
                    Console.WriteLine("PluginDevBuildTool: Failed to send broadcast over websocket");
                    return 1;
                }

                // This value seemed to work instantly every time
                Thread.Sleep(200);

                // A 5 second unload time seems to be a reasonable limit
                const int pollTimeout = 500;
                const int totalTimeout = 5000;
                for (var i = 0; i < totalTimeout / pollTimeout; i++)
                {
                    if (CanOpenExclusively())
                    {
                        File.WriteAllText(unloadedPath, "1");
                        Console.WriteLine($"PluginDevBuildTool: Exclusive access to {pluginName} obtained!");
                        return 0;
                    }
                    Console.WriteLine($"PluginDevBuildTool: Waiting {pollTimeout}ms for exclusive access...");
                    Thread.Sleep(pollTimeout);
                }

                Console.WriteLine("PluginDevBuildTool: Failed to unload plugin...");
                return 1;
            }
            else if (action == "reload")
            {
                if (File.Exists(unloadedPath))
                {
                    File.Delete(unloadedPath);
                    if (!BroadcastIntention())
                        return 1;
                    Console.WriteLine($"PluginDevBuildTool: Requested reload of {pluginName}");
                }
                return 0;
            }
            else
            {
                throw new ArgumentException();
            }
        }
    }
}
