import * as vscode from "vscode";
import { exec } from "child_process";
import * as path from "path";
import * as fs from "fs";
import { promisify } from "util";
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind,
    Trace,
} from "vscode-languageclient/node";

let outputChannel: vscode.OutputChannel | undefined;
let client: LanguageClient | undefined;

const execAsync = promisify(exec);

async function fileError(info: string, settingPath: string) {
    const chosen = await vscode.window.showErrorMessage(
        `${info} not found. Please check your configuration.`,
        "Open Settings",
        "Ignore"
    );

    if (chosen === "Open Settings") {
        vscode.commands.executeCommand("workbench.action.openSettings", settingPath);
    }
}

async function executableExists(nameOrPath: string): Promise<boolean> {
    const isPath = nameOrPath.includes("/") || nameOrPath.includes("\\");

    if (isPath) {
        try {
            fs.accessSync(nameOrPath, fs.constants.F_OK);
            return true;
        } catch {
            return false; // Fix: Ensure we return false if the path is invalid
        }
    }

    const cmd = process.platform === "win32"
        ? `where ${nameOrPath}`
        : `which ${nameOrPath}`;

    try {
        await execAsync(cmd);
        return true;
    } catch {
        return false;
    }
}

export async function activate(context: vscode.ExtensionContext): Promise<void> {
    outputChannel = vscode.window.createOutputChannel("Phasor");

    const config = vscode.workspace.getConfiguration("phasor");
    const lspPath = config.get<string>("lspPath", "phasor-lsp");

    if (await executableExists(lspPath) === false) {
        await fileError("Phasor LSP", "phasor.lspPath");
        return;
    }

    const serverOptions: ServerOptions = {
        command: lspPath,
        transport: TransportKind.stdio,
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: "file", language: "phasor" }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher("**/*.phs")
        },
        outputChannel,
        traceOutputChannel: outputChannel
    };

    client = new LanguageClient("phasorLsp", "Phasor LSP", serverOptions, clientOptions);

    // ENABLE TRACEING
    client.setTrace(Trace.Verbose);

    context.subscriptions.push(client);

    try {
        await client.start();
        outputChannel?.show(true);
        outputChannel?.appendLine("Language server started");
    } catch (err) {
        outputChannel?.show(true);
        outputChannel?.appendLine("Language server failed to start");
        outputChannel?.appendLine(String(err));
        vscode.window.showErrorMessage(`Phasor LSP failed: ${String(err)}`);
    }

    const disposable = vscode.commands.registerCommand("phasor.run", async () => {
        const editor = vscode.window.activeTextEditor;

        if (!editor) {
            vscode.window.showErrorMessage("No active editor found");
            return;
        }

        const document = editor.document;
        if (document.languageId !== "phasor") {
            vscode.window.showErrorMessage("Current file is not a Phasor file");
            return;
        }

        if (document.isDirty) {
            await document.save();
        }

        await runPhasorFile(document.uri.fsPath);
    });

    context.subscriptions.push(disposable);
    context.subscriptions.push(outputChannel);
}

async function runPhasorFile(filePath: string): Promise<void> {
    const config = vscode.workspace.getConfiguration("phasor");
    const jitPath = config.get<string>("jitPath", "phasor");
    const jitArgs = config.get<string[]>("jitArgs", []);
    const clearOutput = config.get<boolean>("clearOutputBeforeRun", true);

    if (clearOutput && outputChannel) {
        outputChannel.clear();
    }

    outputChannel?.show(true);
    outputChannel?.appendLine(`Running: ${path.basename(filePath)}`);
    outputChannel?.appendLine(`Command: ${jitPath} ${jitArgs.join(" ")} "${filePath}"`);
    outputChannel?.appendLine("---");

    if (await executableExists(jitPath) === false) {
        await fileError("Phasor JIT", "phasor.jitPath");
        return;
    }

    const args = [...jitArgs, filePath].map(arg => `"${arg}"`).join(" ");
    const command = `"${jitPath}" ${args}`;
    const cwd = path.dirname(filePath);

    exec(command, { cwd }, (error, stdout, stderr) => {
        if (stdout && outputChannel) {
            outputChannel.appendLine(stdout);
        }

        if (stderr && outputChannel) {
            outputChannel.appendLine("Error output:");
            outputChannel.appendLine(stderr);
        }

        if (error && outputChannel) {
            outputChannel.appendLine("---");
            outputChannel.appendLine(`Program exited with code: ${error.code}`);
            vscode.window.showErrorMessage(`Phasor execution failed: ${error.message}`);
        } else if (outputChannel) {
            outputChannel.appendLine("---");
            outputChannel.appendLine("Execution completed successfully");
        }
    });
}

export async function deactivate(): Promise<void> {
    if (client) {
        await client.stop();
    }
    outputChannel?.dispose();
}