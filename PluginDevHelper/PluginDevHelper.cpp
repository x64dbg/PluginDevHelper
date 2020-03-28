// For some reason WinSock2 has to be first
#ifdef _WIN32
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#endif 

#include "PluginDevHelper.h"

#include <memory>
#include <atomic>
#include <unordered_set>

#include "easywsclient.hpp"
#include "stringutils.h"

using easywsclient::WebSocket;

static HANDLE hWebSocketThread;
static std::atomic_bool bStopWebSocketThread;
static wchar_t pluginDir[MAX_PATH];

#define WEBSOCKET_URL "ws://localhost:4649/PluginDevHelper"

static bool FileExists(const wchar_t* fileName)
{
	DWORD attrib = GetFileAttributesW(fileName);
	return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

static DWORD WINAPI WebSocketThread(LPVOID)
{
	std::unique_ptr<WebSocket> ws(WebSocket::from_url(WEBSOCKET_URL));
	if (!ws)
	{
		dprintf("Failed to connect to %s\n", WEBSOCKET_URL);
		return 0;
	}
	std::unordered_set<std::string> unloadedPlugins;
	while (ws->getReadyState() != WebSocket::CLOSED)
	{
		ws->poll();
		ws->dispatch([&unloadedPlugins](const std::string& message)
		{
			auto colonIdx = message.find(':');
			if (colonIdx != std::string::npos)
			{
				auto plugin = message.substr(colonIdx + 1);
				auto extIdx = plugin.rfind('.');
				if (extIdx != std::string::npos)
				{
					auto actionName = message.substr(0, colonIdx);
					auto pluginName = plugin.substr(0, extIdx);
					auto pluginExt = plugin.substr(extIdx);
#ifdef _WIN64
					if (pluginExt == ".dp64")
#else
					if (pluginExt == ".dp32")
#endif // _WIN64
					{
						auto performAction = [&plugin, &pluginName](const char* commandStr)
						{
							auto pluginPath = pluginDir + Utf8ToUtf16(plugin.c_str());
							if (FileExists(pluginPath.c_str()))
							{
								char cmd[4096] = "";
								_snprintf_s(cmd, _TRUNCATE, "%s \"%s\"", commandStr, pluginName.c_str());
								dprintf("PluginDevHelper: %s\n", cmd);
								// Unload the plugin asynchronously
								DbgCmdExec(cmd);
							}
						};

						if (actionName == "unload")
						{
							unloadedPlugins.insert(plugin);
							performAction("plugunload");
						}
						else if (actionName == "reload")
						{
							auto unloadedItr = unloadedPlugins.find(plugin);
							if (unloadedItr != unloadedPlugins.end())
							{
								unloadedPlugins.erase(unloadedItr);
								performAction("plugload");
							}
							else
							{
								dprintf("Reload of plugin '%s' requested that was not previously unloaded\n", plugin.c_str());
							}
						}
						else
						{
							dprintf("Unknown action '%s'\n", actionName.c_str());
						}
					}
					else
					{
						dprintf("Unsupported extension '%s'\n", pluginExt.c_str());
					}
				}
				else
				{
					dputs("Failed to split extension");
				}
			}
			else
			{
				dputs("Failed to split on ':'");
			}
		});
		if (bStopWebSocketThread && ws->getReadyState() != WebSocket::CLOSING)
			ws->close();
	}
	return 0;
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	// Initialize WinSock
	{
		INT rc;
		WSADATA wsaData;

		rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (rc) {
			printf("WSAStartup Failed.\n");
			return 1;
		}
	}

	// Get plugin directory
	GetModuleFileNameW(GetModuleHandleW(nullptr), pluginDir, _countof(pluginDir));
	auto slash = wcsrchr(pluginDir, L'\\');
	if (slash)
		slash[1] = L'\0';
	wcscat_s(pluginDir, L"plugins\\");

	hWebSocketThread = CreateThread(nullptr, 0, WebSocketThread, nullptr, 0, nullptr);
	return !!hWebSocketThread;
}

//Deinitialize your plugin data here.
void pluginStop()
{
	bStopWebSocketThread = true;
	WaitForSingleObject(hWebSocketThread, INFINITE);
	CloseHandle(hWebSocketThread);
}

//Do GUI/Menu related things here.
void pluginSetup()
{
}
