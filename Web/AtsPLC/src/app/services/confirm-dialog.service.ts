import { Injectable } from '@angular/core';
import { Subject, Observable } from 'rxjs';

export interface ConfirmDialogRequest {
	message: string;
}

@Injectable({
	providedIn: 'root'
})
export class ConfirmDialogService {
	private showRequest = new Subject<ConfirmDialogRequest>();
	private response = new Subject<boolean>();

	get showRequest$(): Observable<ConfirmDialogRequest> {
		return this.showRequest.asObservable();
	}

	show(message: string): Observable<boolean> {
		this.showRequest.next({ message });
		return new Observable<boolean>(observer => {
			const subscription = this.response.subscribe(result => {
				observer.next(result);
				observer.complete();
				subscription.unsubscribe();
			});
		});
	}

	respond(confirmed: boolean): void {
		this.response.next(confirmed);
	}
}
