import { Injectable, signal } from '@angular/core';

@Injectable({
	providedIn: 'root'
})
export class ErrorDialogService {
	readonly message = signal<string | null>(null);

	show(message: string): void {
		this.message.set(message);
	}

	close(): void {
		this.message.set(null);
	}
}
