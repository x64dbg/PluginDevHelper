using System;
using System.Threading;
using WebSocketSharp;
using WebSocketSharp.Server;

namespace PluginDevServer
{
    class BroadcastService : WebSocketBehavior
    {
        protected override void OnMessage(MessageEventArgs e)
        {
            Sessions.Broadcast(e.RawData);
        }
    }
}
