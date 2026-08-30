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

import assert from 'node:assert/strict';
import type { AddressInfo } from 'node:net';
import { test, before, after } from 'node:test';
import { createServer } from 'zorvix';
import registerRoutes from '#phasorweb/routes';
import type { ServerInstance } from 'zorvix';

const SOURCE = "include \"std/io.phs\"; var x: int = 15; var y: int = 12; var z: int = x * y; putf(\"%d * %d = %d\", x, y, z);";
const EXPECTED = {"stdout":"15 * 12 = 180\n","stderr":"","exitCode":0};

let server: ServerInstance;
let baseUrl: string;

before(async () => {
    server = createServer({ port: 0, logging: false });
    await registerRoutes(server);
    const { port } = server.server.address() as AddressInfo;
    baseUrl = `http://localhost:${port}`;
});

after(async () => {
    await server.stop();
});

async function post(path: string, body: string) {
    const res = await fetch(`${baseUrl}${path}`, {
        method:  'POST',
        headers: { 'Content-Type': 'text/plain', 'X-API-Key': process.env.API_KEY ?? '' },
        body,
    });
    return { status: res.status, body: await res.json() };
}

test('POST /run — returns 200 with json body', async () => {
    const { status } = await post('/run', SOURCE);
    assert.equal(status, 200);
});

test('POST /run — stdout matches expected', async () => {
    const { body } = await post('/run', SOURCE);
    assert.equal(body.stdout, EXPECTED.stdout);
});

test('POST /run — stderr matches expected', async () => {
    const { body } = await post('/run', SOURCE);
    assert.equal(body.stderr, EXPECTED.stderr);
});

test('POST /run — exit code matches expected', async () => {
    const { body } = await post('/run', SOURCE);
    assert.equal(body.exitCode, EXPECTED.exitCode);
});

test('POST /run — empty body returns 400', async () => {
    const { status } = await post('/run', '   ');
    assert.equal(status, 400);
});
