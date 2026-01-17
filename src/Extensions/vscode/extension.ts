import * as vscode from "vscode";
import { exec } from "child_process";
import * as path from "path";

let outputChannel: vscode.OutputChannel | undefined;

export function activate(context: vscode.ExtensionContext): void {
    outputChannel = vscode.window.createOutputChannel("Phasor");

    const disposable = vscode.commands.registerCommand("phasor.run", () => {
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
            document.save();
        }

        runPhasorFile(document.uri.fsPath);
    });

    context.subscriptions.push(disposable);
    context.subscriptions.push(outputChannel);
}

function runPhasorFile(filePath: string): void {
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

export function deactivate(): void {
    if (outputChannel) {
        outputChannel.dispose();
    }
}
