import * as vscode from "vscode";
import { exec } from "child_process";
import * as path from "path";
import * as fs from "fs";
import { promisify } from "util";

let outputChannel: vscode.OutputChannel | undefined;

const execAsync = promisify(exec);

async function executableError() {
    const chosen = await vscode.window.showErrorMessage(
        "Phasor JIT executable not found. Please check your configuration.",
        "Open Settings",
        "Ignore"
    );

    if (chosen === "Open Settings") {
        vscode.commands.executeCommand("workbench.action.openSettings", "phasor.jitPath");
    }
}

async function executableExists(nameOrPath: string): Promise<boolean> {
  const isPath = nameOrPath.includes("/") || nameOrPath.includes("\\");

  if (isPath) {
    try {
      fs.accessSync(nameOrPath, fs.constants.F_OK);
      return true;
    } catch {
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
export function activate(context: vscode.ExtensionContext): void {
    outputChannel = vscode.window.createOutputChannel("Phasor");

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

    const args = [...jitArgs, filePath].map(arg => `"${arg}"`).join(" ");
    const command = `"${jitPath}" ${args}`;
    const cwd = path.dirname(filePath);

    if (await executableExists(jitPath) === false) {
        await executableError();
        return;
    }

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
