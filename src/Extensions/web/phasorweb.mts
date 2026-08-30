// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import { serve } from 'zorvix';
import { fileURLToPath } from 'node:url';
import { resolve } from 'node:path';
import { existsSync } from 'node:fs';

const exeName = process.platform === 'win32' ? 'phasor.exe' : 'phasor';
const exePath = resolve(process.cwd(), 'phasor', 'bin', exeName);
process.env["PHASOR_NO_ENV"] = '1';

if (!process.env.API_KEY) {
    console.error("API_KEY is missing!");
    process.exit(1);
}

if (!existsSync(exePath)) {
    console.error(`phasor executable not found at ${exePath}! Please clone and build it using the sandbox preset!`);
    process.exit(1);
}

if (resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
    serve(
        { port: 62811, logging: false, workers: true },
        resolve(process.cwd(), 'dist', 'routes.min.mjs'),
    );
}