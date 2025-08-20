import { SignalingServer } from './signaling_server';

const PORT = 8080;

function main() {
    console.log(`[Main] Starting Signaling Server`);

    const signaling_server = new SignalingServer();

    signaling_server.startServer(PORT);

    process.on('SIGINT', ()=> {
        console.log('[Main] SIGINT');
        signaling_server.stopServer();
        process.exit(0);
    });

    process.on('uncaughtException', (err)=>{
        console.error('[Main] Uncaught Exception: ', err);
        process.exit(1);
    });

    process.on('unhandledRejection', (reason, promise) => {
        console.error('[Main] Unhandled Rejection at:', promise, 'reason:', reason);
    });
}


main();